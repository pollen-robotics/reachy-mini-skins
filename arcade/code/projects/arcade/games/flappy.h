#pragma once
#include "../game.h"
#include "../sound.h"

#define FLAP_W 320
#define FLAP_H 240
#define PIPE_W 20
#define PIPE_GAP 65
#define MAX_PIPES 4
#define FLAP_GRAVITY 8.0f
#define FLAP_FORCE -3.5f

class FlappyGame : public Game {
public:
    float birdX, birdY, birdVY;
    int score, bestScore;
    bool dead, started, exitRequested;
    unsigned long deathTime;
    int propFrame;
    unsigned long propTimer;

    struct Pipe { float x; int gapY; bool scored; bool active; };
    Pipe pipes[MAX_PIPES];
    float pipeSpeed;

    void setup(TFT_eSprite &fb) override {
        exitRequested = false;
        birdX = 60;
        birdY = FLAP_H / 2.0f;
        birdVY = 0;
        score = 0;
        bestScore = 0;
        dead = false;
        started = false;
        deathTime = 0; isGameOver = false;
        propFrame = 0;
        propTimer = 0;
        pipeSpeed = 2.0f;
        for (int i = 0; i < MAX_PIPES; i++) pipes[i].active = false;
    }

    void update(float dt, SensorData &input, TouchData &touch) override {
        // Prop animation
        if (millis() - propTimer > 60) {
            propFrame = (propFrame + 1) % 3;
            propTimer = millis();
        }

        if (dead) {
            if ((input.fireLeft || input.fireRight) && millis() - deathTime > 1500) {
                if (score > bestScore) bestScore = score;
                float oldBest = bestScore;
                restart();
                bestScore = oldBest;
            }
            if (touch.touched && touch.y > 200) exitRequested = true;
            return;
        }

        if (!started) {
            // Hover in place
            birdY = FLAP_H / 2.0f + sinf(millis() / 300.0f) * 5;
            if (input.fireLeft || input.fireRight) {
                started = true;
                birdVY = FLAP_FORCE;
                spawnInitialPipes();
            }
            return;
        }

        // Flap
        if (input.fireLeftPressed() || input.fireRightPressed()) {
            birdVY = FLAP_FORCE;
            SFX::flap();
        }

        // Physics
        birdVY += FLAP_GRAVITY * dt;
        birdY += birdVY;

        // Ceiling
        if (birdY < 8) { birdY = 8; birdVY = 0; }

        // Floor
        if (birdY > FLAP_H - 20) {
            birdY = FLAP_H - 20;
            die();
            return;
        }

        // Pipes
        pipeSpeed = 2.0f + score * 0.08f;
        if (pipeSpeed > 4.5f) pipeSpeed = 4.5f;

        bool needSpawn = true;
        float maxX = 0;
        for (int i = 0; i < MAX_PIPES; i++) {
            if (!pipes[i].active) continue;
            pipes[i].x -= pipeSpeed * dt * 60;

            if (pipes[i].x > maxX) maxX = pipes[i].x;

            // Score
            if (!pipes[i].scored && pipes[i].x + PIPE_W < birdX) {
                pipes[i].scored = true;
                score++;
                SFX::score();
            }

            // Off screen
            if (pipes[i].x < -PIPE_W - 5) {
                pipes[i].active = false;
                continue;
            }

            // Collision
            if (birdX + 8 > pipes[i].x && birdX - 8 < pipes[i].x + PIPE_W) {
                if (birdY - 8 < pipes[i].gapY || birdY + 8 > pipes[i].gapY + PIPE_GAP) {
                    die();
                    return;
                }
            }

            if (pipes[i].x > FLAP_W - 120) needSpawn = false;
        }

        if (needSpawn) spawnPipe();
    }

    void draw(TFT_eSprite &fb) override {
        // Background gradient hint
        for (int y = 0; y < FLAP_H - 18; y += 4) {
            uint8_t b = 10 + y / 8;
            fb.drawFastHLine(0, y, FLAP_W, fb.color565(0, 0, b));
        }

        // Ground
        fb.fillRect(0, FLAP_H - 18, FLAP_W, 18, fb.color565(60, 40, 20));
        fb.drawFastHLine(0, FLAP_H - 18, FLAP_W, fb.color565(80, 60, 30));

        // Pipes
        for (int i = 0; i < MAX_PIPES; i++) {
            if (!pipes[i].active) continue;
            drawPipe(fb, (int)pipes[i].x, pipes[i].gapY);
        }

        // Bird (Reachy Mini with propellers)
        drawBird(fb, (int)birdX, (int)birdY);

        // Score
        fb.setTextColor(TFT_WHITE);
        fb.setTextSize(2);
        char s[8];
        sprintf(s, "%d", score);
        fb.drawCentreString(s, FLAP_W / 2, 10, 1);

        if (!started) {
            fb.setTextColor(TFT_YELLOW);
            fb.setTextSize(1);
            fb.drawCentreString("FIRE TO FLY!", FLAP_W / 2, FLAP_H / 2 + 30, 1);
        }

        if (dead) {
            fb.setTextColor(TFT_RED);
            fb.setTextSize(2);
            fb.drawCentreString("GAME OVER", FLAP_W / 2, 60, 1);
            fb.setTextColor(TFT_CYAN);
            char sc[16];
            sprintf(sc, "SCORE: %d", score);
            fb.drawCentreString(sc, FLAP_W / 2, 90, 1);
            if (bestScore > 0) {
                fb.setTextColor(TFT_YELLOW);
                fb.setTextSize(1);
                sprintf(sc, "BEST: %d", bestScore);
                fb.drawCentreString(sc, FLAP_W / 2, 115, 1);
            }
            fb.setTextColor(TFT_GREEN);
            fb.setTextSize(1);
            fb.drawCentreString("FIRE TO RESTART", FLAP_W / 2, 135, 1);
            fb.setTextColor(0x7BEF);
            fb.drawCentreString("touch bas = menu", FLAP_W / 2, 220, 1);
        }
    }

    bool wantsExit() override { return exitRequested; }
    int getScore() override { return score; }

    void sendState() override {
        char buf[256];
        int pos = 0;
        pos += snprintf(buf+pos, sizeof(buf)-pos,
            "{\"gs\":{\"by\":%.0f,\"bv\":%.1f,\"sc\":%d,\"dd\":%d,\"st\":%d,\"pf\":%d,\"pp\":[",
            birdY, birdVY, score, dead ? 1 : 0, started ? 1 : 0, propFrame);
        bool first = true;
        for (int i = 0; i < MAX_PIPES; i++) {
            if (!pipes[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%d,%d]", pipes[i].x, pipes[i].gapY, pipes[i].scored ? 1 : 0);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "]}}");
        Serial.println(buf);
    }

private:
    void die() {
        dead = true;
        SFX::die();
        deathTime = millis(); isGameOver = true;
    }

    void spawnPipe() {
        for (int i = 0; i < MAX_PIPES; i++) {
            if (!pipes[i].active) {
                int gapY = random(30, FLAP_H - 18 - PIPE_GAP - 10);
                pipes[i] = { (float)FLAP_W + 10, gapY, false, true };
                return;
            }
        }
    }

    void spawnInitialPipes() {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < MAX_PIPES; j++) {
                if (!pipes[j].active) {
                    int gapY = random(30, FLAP_H - 18 - PIPE_GAP - 10);
                    pipes[j] = { (float)(FLAP_W + 40 + i * 160), gapY, false, true };
                    break;
                }
            }
        }
    }

    void drawPipe(TFT_eSprite &fb, int px, int gapY) {
        uint16_t pipeCol = fb.color565(40, 160, 50);
        uint16_t pipeDark = fb.color565(30, 120, 35);
        uint16_t capCol = fb.color565(50, 180, 60);

        // Top pipe
        fb.fillRect(px, 0, PIPE_W, gapY, pipeCol);
        fb.drawRect(px, 0, PIPE_W, gapY, pipeDark);
        // Top cap
        fb.fillRect(px - 3, gapY - 6, PIPE_W + 6, 6, capCol);
        fb.drawRect(px - 3, gapY - 6, PIPE_W + 6, 6, pipeDark);

        // Bottom pipe
        int botY = gapY + PIPE_GAP;
        fb.fillRect(px, botY, PIPE_W, FLAP_H - 18 - botY, pipeCol);
        fb.drawRect(px, botY, PIPE_W, FLAP_H - 18 - botY, pipeDark);
        // Bottom cap
        fb.fillRect(px - 3, botY, PIPE_W + 6, 6, capCol);
        fb.drawRect(px - 3, botY, PIPE_W + 6, 6, pipeDark);
    }

    void drawBird(TFT_eSprite &fb, int bx, int by) {
        uint16_t white = TFT_WHITE;
        uint16_t gray = fb.color565(180, 180, 180);
        uint16_t darkGray = fb.color565(50, 50, 50);

        // Tilt based on velocity
        bool goingUp = birdVY < -1.0f;

        // Body — rounded egg shape
        fb.fillRoundRect(bx - 6, by - 5, 12, 10, 4, white);
        fb.drawRoundRect(bx - 6, by - 5, 12, 10, 4, gray);

        // Head — wider, on top
        fb.fillRoundRect(bx - 7, by - 13, 14, 9, 4, white);
        fb.drawRoundRect(bx - 7, by - 13, 14, 9, 4, gray);

        // Eyes — big round black with bar
        fb.fillCircle(bx - 3, by - 9, 2, TFT_BLACK);
        fb.fillCircle(bx + 3, by - 9, 2, TFT_BLACK);
        fb.drawLine(bx - 1, by - 9, bx + 1, by - 9, TFT_BLACK);

        // Base
        fb.fillRoundRect(bx - 5, by + 4, 10, 3, 1, darkGray);

        // Propellers on top of head!
        int py = by - 14;
        // Mast
        fb.drawLine(bx - 4, py, bx - 4, py - 4, darkGray);
        fb.drawLine(bx + 4, py, bx + 4, py - 4, darkGray);

        // Spinning blades
        if (propFrame == 0) {
            fb.drawFastHLine(bx - 10, py - 4, 7, TFT_WHITE);
            fb.drawFastHLine(bx + 0, py - 4, 7, TFT_WHITE);
            fb.drawFastHLine(bx - 2, py - 4, 5, TFT_WHITE);
        } else if (propFrame == 1) {
            fb.drawFastHLine(bx - 7, py - 4, 4, gray);
            fb.drawFastHLine(bx + 2, py - 4, 4, gray);
        } else {
            fb.drawFastHLine(bx - 9, py - 4, 6, TFT_WHITE);
            fb.drawFastHLine(bx + 1, py - 4, 6, TFT_WHITE);
        }

        // Antennas behind propellers
        fb.drawLine(bx - 4, py - 4, bx - 4, py - 9, gray);
        fb.drawLine(bx + 4, py - 4, bx + 4, py - 9, gray);
    }
};
