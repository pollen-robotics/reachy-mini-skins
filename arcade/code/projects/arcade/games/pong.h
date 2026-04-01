#pragma once
#include "../game.h"
#include "../sound.h"

#define PONG_W 320
#define PONG_H 240
#define PADDLE_W 40
#define PADDLE_H 6
#define BALL_R 3

class PongGame : public Game {
public:
    float playerX, aiX;
    float ballX, ballY, ballVX, ballVY;
    int playerScore, aiScore;
    bool serving;
    bool exitRequested;
    float aiSpeed;
    unsigned long lastBouncePlayer, lastBounceAI;
    unsigned long endTime;

    void setup(TFT_eSprite &fb) override {
        exitRequested = false;
        playerX = PONG_W / 2.0f;
        aiX = PONG_W / 2.0f;
        playerScore = 0;
        aiScore = 0;
        aiSpeed = 2.0f;
        serving = true;
        lastBouncePlayer = 0;
        lastBounceAI = 0;
        endTime = 0; isGameOver = false;
        resetBall(true);
    }

    void update(float dt, SensorData &input, TouchData &touch) override {
        if (playerScore >= 3 || aiScore >= 3) {
            if (endTime == 0) endTime = millis(); isGameOver = true;
            if (millis() - endTime > 1500) {
                if (input.fireLeftPressed() || input.fireRightPressed()) {
                    restart();
                }
                if (touch.touched && touch.y > 200) {
                    exitRequested = true;
                }
            }
            return;
        }

        // Player paddle — roll control
        float targetX = PONG_W / 2.0f + input.roll * -7.0f;
        playerX += (targetX - playerX) * 0.2f;
        playerX = constrain(playerX, PADDLE_W / 2, PONG_W - PADDLE_W / 2);

        if (serving) {
            ballX = playerX;
            ballY = PONG_H - 35;
            if (input.fireLeft || input.fireRight) {
                serving = false;
                ballVY = -3.0f;
                ballVX = (random(2) == 0) ? 1.5f : -1.5f;
                SFX::serve();
            }
            return;
        }

        // Ball movement
        float prevBallY = ballY;
        ballX += ballVX * dt * 60;
        ballY += ballVY * dt * 60;

        // Wall bounce
        if (ballX < BALL_R) { ballX = BALL_R; ballVX = -ballVX; }
        if (ballX > PONG_W - BALL_R) { ballX = PONG_W - BALL_R; ballVX = -ballVX; }

        // Player paddle collision (bottom) — swept check
        float playerPaddleY = PONG_H - 30 - BALL_R;
        if (ballVY > 0 && prevBallY <= playerPaddleY && ballY >= playerPaddleY) {
            if (ballX >= playerX - PADDLE_W / 2 - BALL_R && ballX <= playerX + PADDLE_W / 2 + BALL_R) {
                ballVY = -ballVY;
                ballY = playerPaddleY;
                float offset = (ballX - playerX) / (PADDLE_W / 2.0f);
                ballVX = offset * 3.0f;
                speedUp();
                lastBouncePlayer = millis();
                SFX::bounce();
            }
        }

        // AI paddle collision (top) — swept check
        float aiPaddleY = 30 + PADDLE_H + BALL_R;
        if (ballVY < 0 && prevBallY >= aiPaddleY && ballY <= aiPaddleY) {
            if (ballX >= aiX - PADDLE_W / 2 - BALL_R && ballX <= aiX + PADDLE_W / 2 + BALL_R) {
                ballVY = -ballVY;
                ballY = aiPaddleY;
                float offset = (ballX - aiX) / (PADDLE_W / 2.0f);
                ballVX = offset * 3.0f;
                speedUp();
                lastBounceAI = millis();
                SFX::bounce();
            }
        }

        // Score
        if (ballY > PONG_H + 10) {
            aiScore++;
            serving = true;
            if (aiScore >= 3) SFX::die(); else SFX::miss();
        }
        if (ballY < -10) {
            playerScore++;
            serving = true;
            if (playerScore >= 3) SFX::win(); else SFX::score();
            resetBall(true);
        }

        // AI movement
        float aiTarget = ballX;
        float diff = aiTarget - aiX;
        float maxMove = aiSpeed * dt * 60;
        if (diff > maxMove) diff = maxMove;
        if (diff < -maxMove) diff = -maxMove;
        aiX += diff;
        aiX = constrain(aiX, PADDLE_W / 2, PONG_W - PADDLE_W / 2);
    }

    void draw(TFT_eSprite &fb) override {
        // Center line
        for (int x = 0; x < PONG_W; x += 10) {
            fb.drawFastHLine(x, PONG_H / 2, 5, fb.color565(40, 40, 40));
        }

        if (playerScore >= 3 || aiScore >= 3) {
            drawGameOver(fb);
            return;
        }

        // Player Reachy Mini (bottom)
        drawReachy(fb, (int)playerX, PONG_H - 22, false, lastBouncePlayer);
        // AI evil Reachy (top)
        drawReachy(fb, (int)aiX, 28, true, lastBounceAI);

        // Ball
        fb.fillCircle((int)ballX, (int)ballY, BALL_R, TFT_WHITE);
        fb.drawCircle((int)ballX, (int)ballY, BALL_R + 1, fb.color565(100, 100, 255));

        // Score
        fb.setTextColor(TFT_CYAN);
        fb.setTextSize(2);
        char s[8];
        sprintf(s, "%d", playerScore);
        fb.drawCentreString(s, PONG_W / 2, PONG_H / 2 + 15, 1);
        fb.setTextColor(TFT_RED);
        sprintf(s, "%d", aiScore);
        fb.drawCentreString(s, PONG_W / 2, PONG_H / 2 - 30, 1);

        if (serving) {
            fb.setTextColor(TFT_YELLOW);
            fb.setTextSize(1);
            fb.drawCentreString("FIRE TO SERVE", PONG_W / 2, PONG_H / 2 - 5, 1);
        }
    }

    bool wantsExit() override { return exitRequested; }
    int getScore() override { return playerScore; }

    void sendState() override {
        unsigned long now = millis();
        char buf[160];
        snprintf(buf, sizeof(buf),
            "{\"gs\":{\"px\":%.1f,\"ax\":%.1f,\"bx\":%.1f,\"by\":%.1f,"
            "\"ps\":%d,\"as\":%d,\"sv\":%d,\"bp\":%d,\"ba\":%d}}",
            playerX, aiX, ballX, ballY,
            playerScore, aiScore, serving ? 1 : 0,
            (now - lastBouncePlayer) < 150 ? 1 : 0,
            (now - lastBounceAI) < 150 ? 1 : 0);
        Serial.println(buf);
    }

private:
    void resetBall(bool playerServes) {
        ballX = PONG_W / 2.0f;
        ballY = playerServes ? PONG_H - 35 : 35;
        ballVX = 0;
        ballVY = 0;
    }

    void speedUp() {
        float speed = sqrtf(ballVX * ballVX + ballVY * ballVY);
        if (speed < 5.0f) {
            float factor = 1.05f;
            ballVX *= factor;
            ballVY *= factor;
        }
    }

    void drawReachy(TFT_eSprite &fb, int px, int py, bool evil, unsigned long lastBounce) {
        unsigned long now = millis();
        bool bouncing = (now - lastBounce) < 150;

        if (evil) {
            // Evil Reachy — red tinted, upside down
            fb.fillRoundRect(px - 10, py - 2, 20, 14, 5, TFT_RED);
            fb.fillCircle(px - 4, py + 6, 3, TFT_BLACK);
            fb.fillCircle(px + 4, py + 6, 2, TFT_BLACK);
            fb.drawLine(px - 1, py + 6, px + 2, py + 6, TFT_BLACK);
            // Antennas pointing down
            fb.drawLine(px - 7, py + 12, px - 10, py + 20, fb.color565(200, 100, 100));
            fb.fillCircle(px - 10, py + 21, 2, TFT_RED);
            fb.drawLine(px + 7, py + 12, px + 10, py + 20, fb.color565(200, 100, 100));
            fb.fillCircle(px + 10, py + 21, 2, TFT_RED);
            // Paddle hitbox
            fb.fillRoundRect(px - PADDLE_W / 2, py - 4, PADDLE_W, 3, 1, bouncing ? TFT_YELLOW : fb.color565(180, 50, 50));
        } else {
            // Player Reachy — white, normal
            fb.fillRoundRect(px - 10, py - 12, 20, 14, 5, TFT_WHITE);
            fb.fillCircle(px - 4, py - 6, 3, TFT_BLACK);
            fb.fillCircle(px + 4, py - 6, 2, TFT_BLACK);
            fb.drawLine(px - 1, py - 6, px + 2, py - 6, TFT_BLACK);
            // Antennas pointing up
            fb.drawLine(px - 7, py - 12, px - 10, py - 20, fb.color565(200, 200, 210));
            fb.fillCircle(px - 10, py - 21, 2, TFT_RED);
            fb.drawLine(px + 7, py - 12, px + 10, py - 20, fb.color565(200, 200, 210));
            fb.fillCircle(px + 10, py - 21, 2, TFT_RED);
            // Paddle hitbox
            fb.fillRoundRect(px - PADDLE_W / 2, py + 3, PADDLE_W, 3, 1, bouncing ? TFT_YELLOW : TFT_CYAN);
        }
    }

    void drawGameOver(TFT_eSprite &fb) {
        bool won = playerScore >= 3;
        fb.setTextColor(won ? TFT_GREEN : TFT_RED);
        fb.setTextSize(3);
        fb.drawCentreString(won ? "YOU WIN!" : "YOU LOSE", PONG_W / 2, 70, 1);
        fb.setTextColor(TFT_CYAN);
        fb.setTextSize(2);
        char s[16];
        sprintf(s, "%d - %d", playerScore, aiScore);
        fb.drawCentreString(s, PONG_W / 2, 120, 1);
        fb.setTextColor(TFT_GREEN);
        fb.setTextSize(1);
        fb.drawCentreString("FIRE TO RESTART", PONG_W / 2, 160, 1);
        fb.setTextColor(0x7BEF);
        fb.drawCentreString("touch bas = menu", PONG_W / 2, 220, 1);
    }
};
