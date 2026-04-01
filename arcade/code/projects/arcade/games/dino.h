#pragma once
#include "../game.h"
#include "../sound.h"

#define DINO_W 320
#define DINO_H 240
#define GROUND_Y 195
#define GRAVITY 22.0f
#define JUMP_VEL -7.5f
#define MAX_OBSTACLES 6

class DinoGame : public Game {
public:
    // Duck (player)
    float duckX, duckY, duckVY;
    bool jumping, ducking, holdingJump;
    int legFrame;
    unsigned long legTimer;

    // Game state
    float speed;
    float distance;
    int score;
    bool dead;
    bool exitRequested;
    unsigned long deathTime;

    // Obstacles
    struct Obstacle {
        float x;
        int type; // 0=reachy ground, 1=pollen air, 2=tall reachy
        bool active;
    };
    Obstacle obstacles[MAX_OBSTACLES];
    float nextSpawn;

    // Clouds (decoration)
    struct Cloud { float x, y, speed; };
    Cloud clouds[4];

    void setup(TFT_eSprite &fb) override {
        exitRequested = false;
        duckX = 40;
        duckY = GROUND_Y;
        duckVY = 0;
        jumping = false;
        ducking = false;
        holdingJump = false;
        legFrame = 0;
        legTimer = 0;
        speed = 3.0f;
        distance = 0;
        score = 0;
        dead = false;
        deathTime = 0; isGameOver = false;
        nextSpawn = 150;
        for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
        for (int i = 0; i < 4; i++) {
            clouds[i] = { (float)random(DINO_W), (float)random(20, 70), random(5, 15) / 10.0f };
        }
    }

    void update(float dt, SensorData &input, TouchData &touch) override {
        if (dead) {
            if ((input.fireLeft || input.fireRight) && millis() - deathTime > 1500) {
                restart();
            }
            if (touch.touched && touch.y > 200) {
                exitRequested = true;
            }
            return;
        }

        // Jump — hold antenna for higher jump
        bool fire = input.fireLeft || input.fireRight;
        if (fire && !jumping) {
            jumping = true;
            holdingJump = true;
            duckVY = JUMP_VEL;
            SFX::jump();
        }
        if (!fire) holdingJump = false;

        // Duck (pitch forward)
        ducking = (!jumping && input.pitch > 8.0f);

        // Physics — reduced gravity while holding for higher jump
        if (jumping) {
            float grav = (holdingJump && duckVY < 0) ? GRAVITY * 0.5f : GRAVITY;
            duckVY += grav * dt;
            duckY += duckVY;
            if (duckY >= GROUND_Y) {
                duckY = GROUND_Y;
                duckVY = 0;
                jumping = false;
                holdingJump = false;
            }
        }

        // Leg animation
        if (!jumping && millis() - legTimer > (unsigned long)(80000.0f / (speed * 60))) {
            legFrame = (legFrame + 1) % 2;
            legTimer = millis();
        }

        // Speed up
        speed += 0.2f * dt;
        if (speed > 8.0f) speed = 8.0f;
        distance += speed * dt * 60;
        score = (int)(distance / 10);

        // Spawn obstacles
        nextSpawn -= speed * dt * 60;
        if (nextSpawn <= 0) {
            spawnObstacle();
            nextSpawn = random(180, 400) - speed * 5;
            if (nextSpawn < 120) nextSpawn = 120;
        }

        // Update obstacles
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!obstacles[i].active) continue;
            obstacles[i].x -= speed * dt * 60;
            if (obstacles[i].x < -30) { obstacles[i].active = false; continue; }

            // Collision
            float ox = obstacles[i].x;
            int otype = obstacles[i].type;
            float dw = ducking ? 16 : 12;
            float dh = ducking ? 10 : 34;
            float dy = ducking ? GROUND_Y - 10 : duckY - 34;

            float ow, oh, ocy;
            if (otype == 0) { ow = 14; oh = 18; ocy = GROUND_Y - 18; }       // reachy small
            else if (otype == 1) { ow = 12; oh = 12; ocy = GROUND_Y - 50; }   // pollen air
            else { ow = 16; oh = 28; ocy = GROUND_Y - 28; }                    // tall reachy

            if (duckX + dw > ox - ow/2 && duckX - 6 < ox + ow/2 &&
                dy + dh > ocy && dy < ocy + oh) {
                dead = true;
                deathTime = millis(); isGameOver = true;
                SFX::die();
            }
        }

        // Clouds
        for (int i = 0; i < 4; i++) {
            clouds[i].x -= clouds[i].speed * dt * 60;
            if (clouds[i].x < -20) { clouds[i].x = DINO_W + random(20); clouds[i].y = random(20, 70); }
        }
    }

    void draw(TFT_eSprite &fb) override {
        // Clouds
        for (int i = 0; i < 4; i++) {
            drawCloud(fb, (int)clouds[i].x, (int)clouds[i].y);
        }

        // Ground
        fb.drawFastHLine(0, GROUND_Y + 1, DINO_W, fb.color565(80, 80, 80));
        // Ground texture
        int goff = ((int)distance) % 20;
        for (int x = -goff; x < DINO_W; x += 20) {
            fb.drawFastHLine(x, GROUND_Y + 3, 4, fb.color565(50, 50, 50));
            fb.drawFastHLine(x + 10, GROUND_Y + 5, 3, fb.color565(40, 40, 40));
        }

        // Obstacles
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!obstacles[i].active) continue;
            if (obstacles[i].type == 0) drawReachySmall(fb, (int)obstacles[i].x, GROUND_Y);
            else if (obstacles[i].type == 1) drawDrone(fb, (int)obstacles[i].x, GROUND_Y - 45);
            else drawReachyTall(fb, (int)obstacles[i].x, GROUND_Y);
        }

        // Player duck
        drawDuck(fb, (int)duckX, (int)duckY);

        // HUD
        fb.setTextColor(TFT_WHITE);
        fb.setTextSize(1);
        char s[16];
        sprintf(s, "%05d", score);
        fb.drawString(s, DINO_W - 45, 5, 1);

        if (dead) {
            fb.setTextColor(TFT_RED);
            fb.setTextSize(2);
            fb.drawCentreString("GAME OVER", DINO_W / 2, 70, 1);
            fb.setTextColor(TFT_CYAN);
            fb.setTextSize(2);
            char sc[16];
            sprintf(sc, "SCORE: %d", score);
            fb.drawCentreString(sc, DINO_W / 2, 100, 1);
            fb.setTextColor(TFT_GREEN);
            fb.setTextSize(1);
            fb.drawCentreString("FIRE TO RESTART", DINO_W / 2, 130, 1);
            fb.setTextColor(0x7BEF);
            fb.drawCentreString("touch bas = menu", DINO_W / 2, 220, 1);
        }
    }

    bool wantsExit() override { return exitRequested; }
    int getScore() override { return score; }

    void sendState() override {
        char buf[384];
        int pos = 0;
        pos += snprintf(buf+pos, sizeof(buf)-pos,
            "{\"gs\":{\"dy\":%.0f,\"j\":%d,\"dk\":%d,\"lf\":%d,\"sp\":%.1f,\"di\":%.0f,\"sc\":%d,\"dd\":%d,\"ob\":[",
            duckY, jumping ? 1 : 0, ducking ? 1 : 0, legFrame, speed, distance, score, dead ? 1 : 0);
        bool first = true;
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!obstacles[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%d]", obstacles[i].x, obstacles[i].type);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"cl\":[");
        first = true;
        for (int i = 0; i < 4; i++) {
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f]", clouds[i].x, clouds[i].y);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "]}}");
        Serial.println(buf);
    }

private:
    void spawnObstacle() {
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!obstacles[i].active) {
                int r = random(100);
                int type;
                if (speed < 4.0f) type = 0;
                else if (r < 45) type = 0;
                else if (r < 75) type = 1;
                else type = 2;
                obstacles[i] = { (float)DINO_W + 20, type, true };
                return;
            }
        }
    }

    // --- Mini Duck: bipedal Pollen robot (side profile) ---
    // Wide flat head, black neck, boxy torso with orange stripes,
    // bird-like backward-bending legs, big orange wedge feet
    void drawDuck(TFT_eSprite &fb, int x, int y) {
        uint16_t white = TFT_WHITE;
        uint16_t gray = fb.color565(180, 180, 180);
        uint16_t darkGray = fb.color565(50, 50, 50);
        uint16_t orange = fb.color565(220, 100, 30);

        if (ducking) {
            // Crouched — body low, head forward
            fb.fillRect(x - 5, y - 7, 12, 7, white);
            fb.drawRect(x - 5, y - 7, 12, 7, gray);
            fb.drawFastHLine(x - 5, y - 4, 12, orange);
            // Head forward
            fb.fillRect(x + 7, y - 10, 14, 6, white);
            fb.drawRect(x + 7, y - 10, 14, 6, gray);
            fb.drawFastHLine(x + 7, y - 8, 14, orange);
            fb.fillCircle(x + 18, y - 8, 2, darkGray); // eye
            // Feet
            fb.fillTriangle(x - 4, y, x + 2, y, x - 4, y + 2, orange);
            fb.fillTriangle(x + 5, y, x + 11, y, x + 5, y + 2, orange);
        } else {
            // --- Antenna wire ---
            fb.drawLine(x + 3, y - 34, x + 5, y - 42, gray);

            // --- Head: wide flat box ---
            fb.fillRect(x - 4, y - 34, 16, 7, white);
            fb.drawRect(x - 4, y - 34, 16, 7, gray);
            // Orange stripe on head
            fb.drawFastHLine(x - 4, y - 31, 16, orange);
            // Eye/camera on side (profile)
            fb.fillCircle(x + 10, y - 31, 2, darkGray);
            fb.drawCircle(x + 10, y - 31, 3, gray);

            // --- Neck: dark articulated joint ---
            fb.fillRect(x + 1, y - 27, 5, 4, darkGray);
            fb.fillCircle(x + 3, y - 26, 2, gray);

            // --- Torso: boxy with orange stripe ---
            fb.fillRect(x - 5, y - 23, 13, 12, white);
            fb.drawRect(x - 5, y - 23, 13, 12, gray);
            fb.drawFastHLine(x - 5, y - 18, 13, orange);
            fb.drawFastHLine(x - 5, y - 12, 13, orange);

            // --- Bird legs (digitigrade, backward-bending knee) ---
            if (jumping) {
                // Tucked — thighs angled, shins folded
                // Thigh going down-back
                fb.drawLine(x - 1, y - 11, x - 3, y - 5, white);
                fb.drawLine(x + 0, y - 11, x - 2, y - 5, white);
                fb.drawLine(x + 4, y - 11, x + 2, y - 5, white);
                fb.drawLine(x + 5, y - 11, x + 3, y - 5, white);
                // Knee
                fb.fillCircle(x - 2, y - 5, 2, darkGray);
                fb.fillCircle(x + 3, y - 5, 2, darkGray);
                // Shin angled forward
                fb.drawLine(x - 2, y - 5, x + 0, y - 2, white);
                fb.drawLine(x + 3, y - 5, x + 5, y - 2, white);
            } else if (legFrame == 0) {
                // Back leg (straight-ish)
                fb.fillRect(x - 2, y - 11, 3, 5, white);
                fb.fillCircle(x - 1, y - 6, 2, darkGray); // knee
                fb.drawLine(x - 1, y - 4, x - 3, y - 1, white);
                fb.drawLine(x + 0, y - 4, x - 2, y - 1, white);
                fb.fillTriangle(x - 6, y, x + 0, y, x - 6, y + 2, orange); // foot

                // Front leg (stride forward)
                fb.fillRect(x + 3, y - 11, 3, 5, white);
                fb.fillCircle(x + 4, y - 6, 2, darkGray); // knee
                fb.drawLine(x + 4, y - 4, x + 6, y - 1, white);
                fb.drawLine(x + 5, y - 4, x + 7, y - 1, white);
                fb.fillTriangle(x + 4, y, x + 11, y, x + 4, y + 2, orange); // foot
            } else {
                // Swap
                // Back leg
                fb.fillRect(x + 3, y - 11, 3, 5, white);
                fb.fillCircle(x + 4, y - 6, 2, darkGray);
                fb.drawLine(x + 4, y - 4, x + 2, y - 1, white);
                fb.drawLine(x + 5, y - 4, x + 3, y - 1, white);
                fb.fillTriangle(x - 1, y, x + 5, y, x - 1, y + 2, orange);

                // Front leg
                fb.fillRect(x - 2, y - 11, 3, 5, white);
                fb.fillCircle(x - 1, y - 6, 2, darkGray);
                fb.drawLine(x - 1, y - 4, x + 1, y - 1, white);
                fb.drawLine(x + 0, y - 4, x + 2, y - 1, white);
                fb.fillTriangle(x - 1, y, x + 6, y, x - 1, y + 2, orange);
            }
        }
    }

    // --- Reachy Mini: rounded egg body, big black eyes, spring antennas ---
    void drawReachySmall(TFT_eSprite &fb, int x, int gy) {
        uint16_t gray = fb.color565(200, 200, 200);
        // Base
        fb.fillRoundRect(x - 7, gy - 2, 14, 3, 1, TFT_BLACK);
        // Body — egg shape (wider bottom, narrower top)
        fb.fillRoundRect(x - 6, gy - 12, 12, 11, 5, TFT_WHITE);
        // Head — rounded, slightly wider
        fb.fillRoundRect(x - 7, gy - 20, 14, 9, 4, TFT_WHITE);
        // Eyes — big round black with bar
        fb.fillCircle(x - 3, gy - 16, 3, TFT_BLACK);
        fb.fillCircle(x + 3, gy - 16, 2, TFT_BLACK);
        fb.drawLine(x - 1, gy - 16, x + 1, gy - 16, TFT_BLACK);
        // Antennas — straight
        fb.drawLine(x - 4, gy - 20, x - 4, gy - 28, gray);
        fb.drawLine(x + 4, gy - 20, x + 4, gy - 28, gray);
    }

    // --- Reachy Mini tall variant (stretched, antennas higher) ---
    void drawReachyTall(TFT_eSprite &fb, int x, int gy) {
        uint16_t gray = fb.color565(200, 200, 200);
        // Base
        fb.fillRoundRect(x - 8, gy - 2, 16, 3, 1, TFT_BLACK);
        // Body — taller egg
        fb.fillRoundRect(x - 7, gy - 16, 14, 15, 6, TFT_WHITE);
        // Head
        fb.fillRoundRect(x - 8, gy - 25, 16, 10, 5, TFT_WHITE);
        // Eyes
        fb.fillCircle(x - 3, gy - 21, 3, TFT_BLACK);
        fb.fillCircle(x + 3, gy - 21, 3, TFT_BLACK);
        fb.drawLine(x - 1, gy - 21, x + 1, gy - 21, TFT_BLACK);
        // Antennas — straight, taller
        fb.drawLine(x - 5, gy - 25, x - 5, gy - 36, gray);
        fb.drawLine(x + 5, gy - 25, x + 5, gy - 36, gray);
    }

    // --- Drone (air obstacle) ---
    void drawDrone(TFT_eSprite &fb, int x, int y) {
        uint16_t darkGray = fb.color565(60, 60, 60);
        uint16_t gray = fb.color565(140, 140, 140);
        // Body
        fb.fillRect(x - 4, y - 2, 8, 5, darkGray);
        // Arms
        fb.drawFastHLine(x - 10, y - 1, 7, gray);
        fb.drawFastHLine(x + 4, y - 1, 7, gray);
        // Rotors (spinning)
        int rOff = (millis() / 50) % 2;
        if (rOff == 0) {
            fb.drawFastHLine(x - 13, y - 3, 7, TFT_WHITE);
            fb.drawFastHLine(x + 7, y - 3, 7, TFT_WHITE);
        } else {
            fb.drawFastHLine(x - 11, y - 3, 3, TFT_WHITE);
            fb.drawFastHLine(x + 9, y - 3, 3, TFT_WHITE);
        }
        // LED
        fb.fillCircle(x, y, 1, TFT_RED);
        // Landing skids
        fb.drawFastHLine(x - 5, y + 4, 4, gray);
        fb.drawFastHLine(x + 2, y + 4, 4, gray);
        fb.drawLine(x - 4, y + 3, x - 5, y + 4, gray);
        fb.drawLine(x + 4, y + 3, x + 5, y + 4, gray);
    }

    void drawCloud(TFT_eSprite &fb, int x, int y) {
        uint16_t c = fb.color565(30, 30, 40);
        fb.fillCircle(x, y, 6, c);
        fb.fillCircle(x + 8, y - 2, 5, c);
        fb.fillCircle(x + 14, y, 4, c);
    }
};
