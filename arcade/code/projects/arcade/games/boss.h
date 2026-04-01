#pragma once
#include "../game.h"
#include "../sound.h"

#define BOSS_W 320
#define BOSS_H 240
#define BOSS_MAX_BULLETS 30
#define BOSS_MAX_PLAYER_BULLETS 8
#define BOSS_MAX_EXPLOSIONS 12

class BossGame : public Game {
public:
    // Player
    float px, py;
    int playerHP;
    unsigned long playerHitTime;
    unsigned long fireLeftAnim, fireRightAnim;

    // Boss
    float bx, by, bvx;
    int bossHP, bossMaxHP;
    int phase; // 0=intro, 1=fight phase1, 2=phase2, 3=phase3
    int pattern; // current attack pattern
    unsigned long patternStart;
    unsigned long lastBossShot;
    float bossAnger; // speeds up as HP drops
    bool bossHit;
    unsigned long bossHitTime;

    // Bullets
    struct Bullet { float x, y, vx, vy; bool active; };
    Bullet bossBullets[BOSS_MAX_BULLETS];
    Bullet playerBullets[BOSS_MAX_PLAYER_BULLETS];

    // Explosions
    struct Explosion { float x, y; int frame; bool active; };
    Explosion explosions[BOSS_MAX_EXPLOSIONS];

    // Stars
    struct Star { float x, y, s; };
    Star stars[40];

    bool dead, won, exitRequested;
    unsigned long endTime;
    int score;

    void setup(TFT_eSprite &fb) override {
        exitRequested = false;
        px = BOSS_W / 2.0f; py = 200;
        playerHP = 100;
        playerHitTime = 0;
        fireLeftAnim = 0; fireRightAnim = 0;

        bx = BOSS_W / 2.0f; by = -40;
        bossMaxHP = 200; bossHP = bossMaxHP;
        bvx = 0.8f;
        phase = 0; pattern = 0;
        patternStart = millis();
        lastBossShot = 0;
        bossAnger = 1.0f;
        bossHit = false; bossHitTime = 0;

        for (int i = 0; i < BOSS_MAX_BULLETS; i++) bossBullets[i].active = false;
        for (int i = 0; i < BOSS_MAX_PLAYER_BULLETS; i++) playerBullets[i].active = false;
        for (int i = 0; i < BOSS_MAX_EXPLOSIONS; i++) explosions[i].active = false;
        for (int i = 0; i < 40; i++) stars[i] = { (float)random(BOSS_W), (float)random(BOSS_H), random(10,40)/10.0f };

        dead = false; won = false;
        endTime = 0; isGameOver = false; score = 0;
    }

    void update(float dt, SensorData &input, TouchData &touch) override {
        updateStars(dt);
        updateExplosions();

        if (dead || won) {
            if ((input.fireLeft || input.fireRight) && millis() - endTime > 1500) {
                restart();
            }
            if (touch.touched && touch.y > 200) exitRequested = true;
            return;
        }

        // --- Intro: boss descends ---
        if (phase == 0) {
            by += 40 * dt;
            if (by >= 45) { by = 45; phase = 1; patternStart = millis(); pattern = 0; }
            return;
        }

        // --- Player movement ---
        float targetX = BOSS_W / 2.0f + input.roll * -7.0f;
        float targetY = 200.0f - input.pitch * 2.0f;
        px += (targetX - px) * 0.15f;
        py += (targetY - py) * 0.15f;
        px = constrain(px, 12, BOSS_W - 12);
        py = constrain(py, 100, BOSS_H - 12);

        // --- Player shoot ---
        if (input.fireLeftPressed()) {
            spawnPlayerBullet(px - 8, py - 5);
            fireLeftAnim = millis();
            SFX::shoot();
        }
        if (input.fireRightPressed()) {
            spawnPlayerBullet(px + 8, py - 5);
            fireRightAnim = millis();
            SFX::shoot();
        }

        // --- Boss AI ---
        bossAnger = 1.0f + (1.0f - (float)bossHP / bossMaxHP) * 1.5f;

        // Boss horizontal movement
        bx += bvx * bossAnger * dt * 60;
        if (bx < 40) { bx = 40; bvx = fabsf(bvx); }
        if (bx > BOSS_W - 40) { bx = BOSS_W - 40; bvx = -fabsf(bvx); }

        // Phase transitions
        if (bossHP < bossMaxHP * 0.66f && phase < 2) { phase = 2; pattern = 0; patternStart = millis(); SFX::phase(); }
        if (bossHP < bossMaxHP * 0.33f && phase < 3) { phase = 3; pattern = 0; patternStart = millis(); SFX::phase(); }

        // Attack patterns
        unsigned long patternAge = millis() - patternStart;
        unsigned long now = millis();
        unsigned long cooldown = (unsigned long)(800 / bossAnger);

        if (patternAge > 3000) {
            pattern = (pattern + 1) % (phase + 2);
            patternStart = millis();
        }

        if (now - lastBossShot > cooldown) {
            lastBossShot = now;
            switch (pattern) {
                case 0: attackSpread(); break;
                case 1: attackAimed(); break;
                case 2: attackSweep(); break;
                case 3: attackSpiral(); break;
            }
        }

        // --- Update bullets ---
        updateBossBullets(dt);
        updatePlayerBullets(dt);

        // --- Check boss dead ---
        if (bossHP <= 0) {
            won = true;
            endTime = millis(); isGameOver = true;
            score = playerHP * 10;
            SFX::win();
            // Big explosion
            for (int i = 0; i < 8; i++) {
                spawnExplosion(bx + random(-20, 20), by + random(-15, 15));
            }
        }
    }

    void draw(TFT_eSprite &fb) override {
        drawStars(fb);

        // Boss bullets
        for (int i = 0; i < BOSS_MAX_BULLETS; i++) {
            if (!bossBullets[i].active) continue;
            fb.fillCircle((int)bossBullets[i].x, (int)bossBullets[i].y, 2, TFT_RED);
        }

        // Player bullets
        for (int i = 0; i < BOSS_MAX_PLAYER_BULLETS; i++) {
            if (!playerBullets[i].active) continue;
            fb.fillRect((int)playerBullets[i].x - 1, (int)playerBullets[i].y - 3, 3, 6, TFT_GREEN);
        }

        // Explosions
        for (int i = 0; i < BOSS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) continue;
            int r = explosions[i].frame * 2 + 2;
            uint8_t a = 255 - explosions[i].frame * 28;
            fb.drawCircle((int)explosions[i].x, (int)explosions[i].y, r, fb.color565(a, a/2, 0));
            fb.drawCircle((int)explosions[i].x, (int)explosions[i].y, r/2, TFT_YELLOW);
        }

        if (!won) drawBoss(fb);
        if (!dead && !won) drawPlayer(fb);

        // HUD
        drawHUD(fb);

        if (dead) drawGameOver(fb);
        if (won) drawVictory(fb);
    }

    bool wantsExit() override { return exitRequested; }
    int getScore() override { return score; }

    void sendState() override {
        unsigned long now = millis();
        char buf[768];
        int pos = 0;
        pos += snprintf(buf+pos, sizeof(buf)-pos,
            "{\"gs\":{\"px\":%.0f,\"py\":%.0f,\"php\":%d,\"bx\":%.0f,\"by\":%.0f,\"bhp\":%d,\"ph\":%d,"
            "\"fl\":%d,\"fr\":%d,\"phi\":%d,\"bhi\":%d,\"dd\":%d,\"wn\":%d,\"sc\":%d,\"bb\":[",
            px, py, playerHP, bx, by, bossHP, phase,
            (now - fireLeftAnim) < 150 ? 1 : 0,
            (now - fireRightAnim) < 150 ? 1 : 0,
            (now - playerHitTime) < 150 ? 1 : 0,
            (now - bossHitTime) < 80 ? 1 : 0,
            dead ? 1 : 0, won ? 1 : 0, score);
        bool first = true;
        for (int i = 0; i < BOSS_MAX_BULLETS; i++) {
            if (!bossBullets[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f]", bossBullets[i].x, bossBullets[i].y);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"pb\":[");
        first = true;
        for (int i = 0; i < BOSS_MAX_PLAYER_BULLETS; i++) {
            if (!playerBullets[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f]", playerBullets[i].x, playerBullets[i].y);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"ex\":[");
        first = true;
        for (int i = 0; i < BOSS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f,%d]", explosions[i].x, explosions[i].y, explosions[i].frame);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "]}}");
        Serial.println(buf);
    }

private:
    // --- Attack patterns ---
    void attackSpread() {
        float angles[] = { -0.4f, -0.2f, 0.0f, 0.2f, 0.4f };
        int n = (phase >= 3) ? 5 : 3;
        int start = (phase >= 3) ? 0 : 1;
        for (int i = start; i < start + n; i++) {
            spawnBossBullet(bx, by + 20, sinf(angles[i]) * 2.0f, 2.5f);
        }
    }

    void attackAimed() {
        float dx = px - bx, dy = py - by;
        float len = sqrtf(dx*dx + dy*dy);
        if (len < 1) return;
        float spd = 2.5f + (phase - 1) * 0.5f;
        spawnBossBullet(bx, by + 20, dx/len * spd, dy/len * spd);
        if (phase >= 3) {
            spawnBossBullet(bx - 10, by + 20, dx/len * spd, dy/len * spd);
            spawnBossBullet(bx + 10, by + 20, dx/len * spd, dy/len * spd);
        }
    }

    void attackSweep() {
        float t = (millis() - patternStart) / 500.0f;
        float angle = sinf(t * 2.0f) * 1.2f;
        spawnBossBullet(bx, by + 20, sinf(angle) * 3.0f, cosf(angle) * 2.5f);
    }

    void attackSpiral() {
        float t = (millis() - patternStart) / 200.0f;
        spawnBossBullet(bx, by + 15, sinf(t) * 2.5f, cosf(t) * 1.5f + 1.5f);
    }

    // --- Spawners ---
    void spawnBossBullet(float x, float y, float vx, float vy) {
        for (int i = 0; i < BOSS_MAX_BULLETS; i++) {
            if (!bossBullets[i].active) { bossBullets[i] = {x, y, vx, vy, true}; return; }
        }
    }

    void spawnPlayerBullet(float x, float y) {
        for (int i = 0; i < BOSS_MAX_PLAYER_BULLETS; i++) {
            if (!playerBullets[i].active) { playerBullets[i] = {x, y, 0, -6.0f, true}; return; }
        }
    }

    void spawnExplosion(float x, float y) {
        for (int i = 0; i < BOSS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) { explosions[i] = {x, y, 0, true}; return; }
        }
    }

    // --- Updates ---
    void updateBossBullets(float dt) {
        for (int i = 0; i < BOSS_MAX_BULLETS; i++) {
            if (!bossBullets[i].active) continue;
            bossBullets[i].x += bossBullets[i].vx * dt * 60;
            bossBullets[i].y += bossBullets[i].vy * dt * 60;
            if (bossBullets[i].y > BOSS_H + 5 || bossBullets[i].y < -5 ||
                bossBullets[i].x < -5 || bossBullets[i].x > BOSS_W + 5) {
                bossBullets[i].active = false; continue;
            }
            // Hit player
            float dx = bossBullets[i].x - px, dy = bossBullets[i].y - py;
            if (dx*dx + dy*dy < 80) {
                bossBullets[i].active = false;
                playerHP -= 8;
                playerHitTime = millis();
                if (playerHP <= 0) { playerHP = 0; dead = true; endTime = millis(); isGameOver = true; SFX::die(); } else { SFX::hit(); }
            }
        }
    }

    void updatePlayerBullets(float dt) {
        for (int i = 0; i < BOSS_MAX_PLAYER_BULLETS; i++) {
            if (!playerBullets[i].active) continue;
            playerBullets[i].y += playerBullets[i].vy * dt * 60;
            if (playerBullets[i].y < -5) { playerBullets[i].active = false; continue; }
            // Hit boss
            float dx = playerBullets[i].x - bx, dy = playerBullets[i].y - by;
            if (fabsf(dx) < 22 && fabsf(dy) < 18) {
                playerBullets[i].active = false;
                bossHP -= 5;
                bossHit = true;
                bossHitTime = millis();
                spawnExplosion(playerBullets[i].x, playerBullets[i].y);
                SFX::boss_hit();
                if (bossHP < 0) bossHP = 0;
            }
        }
    }

    void updateStars(float dt) {
        for (int i = 0; i < 40; i++) {
            stars[i].y += stars[i].s * dt * 60;
            if (stars[i].y > BOSS_H) { stars[i].y = 0; stars[i].x = random(BOSS_W); }
        }
    }

    void updateExplosions() {
        for (int i = 0; i < BOSS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) continue;
            explosions[i].frame++;
            if (explosions[i].frame > 8) explosions[i].active = false;
        }
    }

    // --- Draw ---
    void drawStars(TFT_eSprite &fb) {
        for (int i = 0; i < 40; i++) {
            uint8_t b = (uint8_t)(stars[i].s * 25);
            fb.drawPixel(stars[i].x, stars[i].y, fb.color565(b, b, b));
        }
    }

    void drawPlayer(TFT_eSprite &fb) {
        bool hit = (millis() - playerHitTime) < 100;
        int ipx = (int)px, ipy = (int)py;

        // Reachy Mini player — same as spaceship
        uint16_t headCol = hit ? TFT_RED : TFT_WHITE;
        fb.fillRoundRect(ipx - 10, ipy - 12, 20, 14, 5, headCol);
        fb.fillCircle(ipx - 4, ipy - 6, 3, TFT_BLACK);
        fb.fillCircle(ipx + 4, ipy - 6, 2, TFT_BLACK);
        fb.drawLine(ipx - 1, ipy - 6, ipx + 2, ipy - 6, TFT_BLACK);

        unsigned long now = millis();
        bool lF = (now - fireLeftAnim) < 150;
        bool rF = (now - fireRightAnim) < 150;
        int lO = lF ? 3 : 0, rO = rF ? 3 : 0;
        fb.drawLine(ipx - 7, ipy - 12, ipx - 10, ipy - 20 - lO, fb.color565(200,200,210));
        fb.fillCircle(ipx - 10, ipy - 21 - lO, 2, lF ? TFT_YELLOW : TFT_RED);
        if (lF) fb.fillCircle(ipx - 10, ipy - 21 - lO, 4, fb.color565(255,200,0));
        fb.drawLine(ipx + 7, ipy - 12, ipx + 10, ipy - 20 - rO, fb.color565(200,200,210));
        fb.fillCircle(ipx + 10, ipy - 21 - rO, 2, rF ? TFT_YELLOW : TFT_RED);
        if (rF) fb.fillCircle(ipx + 10, ipy - 21 - rO, 4, fb.color565(255,200,0));
    }

    void drawBoss(TFT_eSprite &fb) {
        bool hit = (millis() - bossHitTime) < 80;
        int ibx = (int)bx, iby = (int)by;

        uint16_t bodyCol = hit ? fb.color565(255, 150, 150) : TFT_WHITE;
        uint16_t gray = fb.color565(180, 180, 180);
        uint16_t darkGray = fb.color565(50, 50, 50);

        // Glow/aura based on phase
        uint16_t auraCol = (phase >= 3) ? TFT_RED : (phase >= 2) ? TFT_YELLOW : fb.color565(100, 100, 255);
        fb.drawRoundRect(ibx - 23, iby - 19, 46, 38, 8, auraCol);

        // Big body — egg shape
        fb.fillRoundRect(ibx - 18, iby - 8, 36, 26, 10, bodyCol);
        fb.drawRoundRect(ibx - 18, iby - 8, 36, 26, 10, gray);

        // Base
        fb.fillRoundRect(ibx - 14, iby + 16, 28, 5, 2, darkGray);

        // Head — big rounded
        fb.fillRoundRect(ibx - 20, iby - 16, 40, 18, 8, bodyCol);
        fb.drawRoundRect(ibx - 20, iby - 16, 40, 18, 8, gray);

        // Big eyes — menacing
        uint16_t eyeCol = (phase >= 3) ? TFT_RED : TFT_BLACK;
        fb.fillCircle(ibx - 8, iby - 8, 5, eyeCol);
        fb.fillCircle(ibx + 8, iby - 8, 4, eyeCol);
        fb.drawLine(ibx - 3, iby - 8, ibx + 4, iby - 8, eyeCol);

        // Angry eyebrows in later phases
        if (phase >= 2) {
            fb.drawLine(ibx - 13, iby - 15, ibx - 4, iby - 13, TFT_RED);
            fb.drawLine(ibx + 13, iby - 15, ibx + 4, iby - 13, TFT_RED);
        }

        // Antennas
        fb.drawLine(ibx - 12, iby - 16, ibx - 14, iby - 28, gray);
        fb.drawLine(ibx + 12, iby - 16, ibx + 14, iby - 28, gray);
        // Antenna tips glow with phase
        fb.fillCircle(ibx - 14, iby - 29, 3, auraCol);
        fb.fillCircle(ibx + 14, iby - 29, 3, auraCol);
    }

    void drawHUD(TFT_eSprite &fb) {
        // Player HP
        fb.drawRect(5, BOSS_H - 12, 102, 8, TFT_WHITE);
        uint16_t hpCol = (playerHP > 50) ? TFT_GREEN : (playerHP > 25) ? TFT_YELLOW : TFT_RED;
        fb.fillRect(6, BOSS_H - 11, playerHP, 6, hpCol);

        // Boss HP
        if (phase > 0 && !won) {
            int bw = 200;
            int bx0 = (BOSS_W - bw) / 2;
            fb.drawRect(bx0, 3, bw + 2, 8, TFT_WHITE);
            int fill = (int)((float)bossHP / bossMaxHP * bw);
            uint16_t bCol = (bossHP > bossMaxHP * 0.5f) ? TFT_RED : (bossHP > bossMaxHP * 0.25f) ? TFT_MAGENTA : fb.color565(255, 50, 50);
            fb.fillRect(bx0 + 1, 4, fill, 6, bCol);

            fb.setTextColor(TFT_WHITE);
            fb.setTextSize(1);
            fb.drawString("BOSS", bx0 - 30, 3, 1);
        }
    }

    void drawGameOver(TFT_eSprite &fb) {
        fb.setTextColor(TFT_RED); fb.setTextSize(2);
        fb.drawCentreString("GAME OVER", BOSS_W / 2, 80, 1);
        fb.setTextColor(TFT_GREEN); fb.setTextSize(1);
        fb.drawCentreString("FIRE TO RETRY", BOSS_W / 2, 115, 1);
        fb.setTextColor(0x7BEF);
        fb.drawCentreString("touch bas = menu", BOSS_W / 2, 220, 1);
    }

    void drawVictory(TFT_eSprite &fb) {
        fb.setTextColor(TFT_GREEN); fb.setTextSize(2);
        fb.drawCentreString("VICTORY!", BOSS_W / 2, 60, 1);
        fb.setTextColor(TFT_CYAN);
        char sc[20];
        sprintf(sc, "SCORE: %d", score);
        fb.drawCentreString(sc, BOSS_W / 2, 90, 1);
        fb.setTextColor(TFT_YELLOW); fb.setTextSize(1);
        char hp[20];
        sprintf(hp, "HP LEFT: %d%%", playerHP);
        fb.drawCentreString(hp, BOSS_W / 2, 115, 1);
        fb.setTextColor(TFT_GREEN);
        fb.drawCentreString("FIRE TO REPLAY", BOSS_W / 2, 140, 1);
        fb.setTextColor(0x7BEF);
        fb.drawCentreString("touch bas = menu", BOSS_W / 2, 220, 1);
    }
};
