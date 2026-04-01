#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <ArduinoJson.h>
#include "game.h"
#include "scores.h"
#include "sound.h"
#include "name_entry.h"
#include "menu.h"
#include "games/spaceship.h"
#include "games/pong.h"
#include "games/dino.h"
#include "games/flappy.h"
#include "games/boss.h"
#include "games/rhythm.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define SERIAL_BAUD 2000000

// Touch - exactement comme le calibrate
#define XPT_CLK  25
#define XPT_MISO 39
#define XPT_MOSI 32
#define XPT_CS   33
#define XPT_IRQ  36

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite fb = TFT_eSprite(&tft);
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT_CS, XPT_IRQ);

Menu menu;
ScoreManager scoreMgr;
NameEntry nameEntry;
SensorData sensor;
TouchData touch;

Game *currentGame = nullptr;
SpaceshipGame spaceshipGame;
PongGame pongGame;
DinoGame dinoGame;
FlappyGame flappyGame;
BossGame bossGame;
RhythmGame rhythmGame;

unsigned long lastFrame = 0;
unsigned long lastTouchTime = 0;
bool lastLeaderboardState = false;
#define TOUCH_DEBOUNCE 300

Game* createSpaceship() { return &spaceshipGame; }
Game* createPong() { return &pongGame; }
Game* createDino() { return &dinoGame; }
Game* createFlappy() { return &flappyGame; }
Game* createBoss() { return &bossGame; }
Game* createRhythm() { return &rhythmGame; }

// Serial input
String serialBuf;
void readSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            JsonDocument doc;
            if (deserializeJson(doc, serialBuf) == DeserializationError::Ok) {
                // Sensor data from bridge
                if (doc.containsKey("roll")) {
                    sensor.rawRoll  = doc["roll"]  | 0.0f;
                    sensor.rawPitch = doc["pitch"] | 0.0f;
                    sensor.rawYaw   = doc["yaw"]   | 0.0f;
                    sensor.applyCalibration();
                    sensor.fireLeft  = doc["fire_left"]  | false;
                    sensor.fireRight = doc["fire_right"] | false;
                }
                // Score sync from bridge (full or single update)
                if (doc.containsKey("scores_sync")) {
                    JsonArray arr = doc["scores_sync"].as<JsonArray>();
                    for (JsonObject entry : arr) {
                        int gi = entry["game"] | -1;
                        int sc = entry["score"] | 0;
                        const char* nm = entry["name"] | "AAAA";
                        scoreMgr.receiveUpdate(gi, sc, nm);
                    }
                }
                if (doc.containsKey("score_update")) {
                    JsonObject u = doc["score_update"].as<JsonObject>();
                    int gi = u["game"] | -1;
                    int sc = u["score"] | 0;
                    const char* nm = u["name"] | "AAAA";
                    scoreMgr.receiveUpdate(gi, sc, nm);
                }
                if (doc.containsKey("ping")) {
                    Serial.println("{\"pong\":\"arcade\"}");
                }
            }
            serialBuf = "";
        } else {
            serialBuf += c;
        }
    }
}

void readTouch() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        touch.rawX = p.x;
        touch.rawY = p.y;
        touch.x = map(p.x, 430, 3570, 0, 319);
        touch.y = map(p.y, 500, 3550, 0, 239);
        touch.x = constrain(touch.x, 0, 319);
        touch.y = constrain(touch.y, 0, 239);
        touch.touched = true;
        lastTouchTime = millis();
    } else if (millis() - lastTouchTime > 150) {
        touch.touched = false;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);

    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);

    // TFT d'abord
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Touch ensuite - exactement comme calibrate
    touchSPI.begin(XPT_CLK, XPT_MISO, XPT_MOSI, XPT_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);

    fb.setColorDepth(8);
    if (!fb.createSprite(SCREEN_W, SCREEN_H)) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("SPRITE FAIL", 160, 110, 1);
        while (1) delay(1000);
    }

    scoreMgr.load();
    menu.scoreMgr = &scoreMgr;

    menu.addGame("Spaceship", 0xFD20, iconSpaceship, createSpaceship);
    menu.addGame("Pong", TFT_YELLOW, iconPong, createPong);
    menu.addGame("Duck Run", TFT_RED, iconDino, createDino);
    menu.addGame("Flappy", TFT_GREEN, iconFlappy, createFlappy);
    menu.addGame("Boss", TFT_MAGENTA, iconBoss, createBoss);
    menu.addGame("Rhythm", TFT_CYAN, iconRhythm, createRhythm);

    menu.menuEnterTime = millis();
    lastFrame = millis();
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastFrame) / 1000.0f;
    lastFrame = now;
    if (dt > 0.05f) dt = 0.05f;

    readSerial();
    readTouch();

    fb.fillSprite(TFT_BLACK);

    // --- Name entry mode ---
    if (nameEntry.active) {
        // Draw the game's game over / victory screen behind
        if (currentGame) {
            currentGame->draw(fb);
        }
        if (nameEntry.update(sensor)) {
            // Done entering name — save and return to game over screen
            scoreMgr.save(nameEntry.gameIndex, nameEntry.finalScore, nameEntry.letters);
            // Game stays in its game over state, player can then restart/exit normally
        }
        nameEntry.draw(fb);
        sensor.latch();
        fb.pushSprite(0, 0);
        return;
    }

    // --- Menu ---
    if (currentGame == nullptr) {
        menu.draw(fb);

        // Send leaderboard state changes to web
        if (menu.showLeaderboard != lastLeaderboardState) {
            lastLeaderboardState = menu.showLeaderboard;
            if (menu.showLeaderboard) {
                Serial.println("{\"show_scores\":true}");
            } else {
                Serial.println("{\"hide_scores\":true}");
            }
        }

        if (touch.touched && (now - lastTouchTime < 300)) {
            int sel = menu.handleTouch(touch);
            if (sel >= 0 && menu.entries[sel].create != nullptr) {
                touch.touched = false;
                SFX::select();
                SFX::gameStart(menu.entries[sel].name);
                sensor.calibrate();
                currentGame = menu.entries[sel].create();
                currentGame->_fb = &fb;
                currentGame->gameIndex = sel;
                currentGame->scores = &scoreMgr;
                currentGame->setup(fb);
            }
        }
    }
    // --- Game ---
    else {
        currentGame->update(dt, sensor, touch);
        currentGame->draw(fb);
        currentGame->sendState();
        sensor.latch();

        // Show name entry as soon as game over with new high score
        if (currentGame->isGameOver && !nameEntry.active) {
            int gi = currentGame->gameIndex;
            int sc = currentGame->getScore();
            if (sc > 0 && scoreMgr.isNewBest(gi, sc)) {
                nameEntry.start(gi, sc, scoreMgr.lastName);
            }
        }

        if (currentGame->wantsExit()) {
            SFX::gameStop();
            currentGame = nullptr;
            menu.menuEnterTime = millis();
        }
    }

    fb.pushSprite(0, 0);
}
