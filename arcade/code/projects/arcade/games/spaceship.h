#pragma once
#include "../game.h"
#include "../sound.h"

#define SS_MAX_STARS 80
#define SS_MAX_ENEMIES 8
#define SS_MAX_PROJECTILES 10
#define SS_MAX_ENEMY_BULLETS 15
#define SS_MAX_EXPLOSIONS 20
#define SS_FIRE_ANIM_DURATION 150
#define SS_W 320
#define SS_H 240

class SpaceshipGame : public Game {
public:
    // --- Player ---
    float playerX, playerY;
    int playerHP, score;
    unsigned long fireLeftAnim, fireRightAnim;
    unsigned long playerHitTime;
    unsigned long deathTime;
    bool exitRequested;

    // --- Stars ---
    struct Star { float x, y, speed; };
    Star stars[SS_MAX_STARS];

    // --- Projectiles ---
    struct Projectile { float x, y; bool active; };
    Projectile projectiles[SS_MAX_PROJECTILES];

    // --- Enemies ---
    enum EnemyType { BASIC, FAST, HEAVY };
    struct Enemy { float x, y, vx; int hp; EnemyType type; bool active; unsigned long lastShot; unsigned long hitTime; };
    Enemy enemies[SS_MAX_ENEMIES];

    // --- Enemy bullets ---
    struct EBullet { float x, y, vx, vy; bool active; };
    EBullet eBullets[SS_MAX_ENEMY_BULLETS];

    // --- Explosions ---
    struct Explosion { float x, y; int frame; bool active; };
    Explosion explosions[SS_MAX_EXPLOSIONS];

    // --- Waves ---
    int wave, enemiesSpawned, enemiesToSpawn;
    bool waveActive;
    unsigned long nextWaveTime, nextSpawnTime, waveAnnounceEnd;

    // --- Timing ---
    float dt;

    uint16_t enemyColor(EnemyType t) {
        switch (t) {
            case BASIC: return TFT_RED;
            case FAST:  return 0xFD20; // orange
            case HEAVY: return TFT_MAGENTA;
        }
        return TFT_RED;
    }

    void setup(TFT_eSprite &fb) override {
        exitRequested = false;
        playerX = 160; playerY = 180;
        playerHP = 100; score = 0;
        fireLeftAnim = 0; fireRightAnim = 0;
        playerHitTime = 0;
        deathTime = 0; isGameOver = false;
        wave = 0; enemiesSpawned = 0; enemiesToSpawn = 0;
        waveActive = false;
        nextWaveTime = millis() + 2000;
        waveAnnounceEnd = 0;
        for (int i = 0; i < SS_MAX_STARS; i++) {
            stars[i] = { (float)random(SS_W), (float)random(SS_H), random(20,80)/10.0f };
        }
        for (int i = 0; i < SS_MAX_PROJECTILES; i++) projectiles[i].active = false;
        for (int i = 0; i < SS_MAX_ENEMIES; i++) enemies[i].active = false;
        for (int i = 0; i < SS_MAX_ENEMY_BULLETS; i++) eBullets[i].active = false;
        for (int i = 0; i < SS_MAX_EXPLOSIONS; i++) explosions[i].active = false;
    }

    void update(float _dt, SensorData &input, TouchData &touch) override {
        dt = _dt;
        updateStars();

        if (playerHP > 0) {
            updatePlayer(input);
            updateProjectiles();
            updateEnemies();
            updateEBullets();
            updateExplosions();
            updateWaves();
        } else if (millis() - deathTime > 1500) {
            if (input.fireLeftPressed() || input.fireRightPressed()) {
                restart();
            }
            if (touch.touched && touch.y > 200) {
                exitRequested = true;
            }
        }
    }

    void draw(TFT_eSprite &fb) override {
        drawStars(fb);
        if (playerHP > 0) {
            drawEnemies(fb);
            drawProjectiles(fb);
            drawEBullets(fb);
            drawExplosions(fb);
            drawPlayer(fb);
            drawHUD(fb);
        } else {
            drawGameOver(fb);
        }
    }

    bool wantsExit() override { return exitRequested; }
    int getScore() override { return score; }

    void sendState() override {
        unsigned long now = millis();
        char buf[768];
        int pos = 0;
        pos += snprintf(buf+pos, sizeof(buf)-pos,
            "{\"gs\":{\"px\":%.0f,\"py\":%.0f,\"hp\":%d,\"sc\":%d,\"w\":%d,\"wa\":%d,\"fl\":%d,\"fr\":%d,\"ph\":%d,\"go\":%d,\"en\":[",
            playerX, playerY, playerHP, score, wave,
            (waveAnnounceEnd > 0 && now < waveAnnounceEnd) ? 1 : 0,
            (now - fireLeftAnim) < 150 ? 1 : 0,
            (now - fireRightAnim) < 150 ? 1 : 0,
            (now - playerHitTime) < 150 ? 1 : 0,
            (playerHP <= 0) ? 1 : 0);
        bool first = true;
        for (int i = 0; i < SS_MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f,%d,%d]",
                enemies[i].x, enemies[i].y, (int)enemies[i].type,
                (now - enemies[i].hitTime) < 120 ? 1 : 0);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"pr\":[");
        first = true;
        for (int i = 0; i < SS_MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f]", projectiles[i].x, projectiles[i].y);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"eb\":[");
        first = true;
        for (int i = 0; i < SS_MAX_ENEMY_BULLETS; i++) {
            if (!eBullets[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f]", eBullets[i].x, eBullets[i].y);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"ex\":[");
        first = true;
        for (int i = 0; i < SS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%.0f,%d]", explosions[i].x, explosions[i].y, explosions[i].frame);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "]}}");
        Serial.println(buf);
    }

private:
    // --- Spawn helpers ---
    void spawnProjectile(float x, float y) {
        for (int i = 0; i < SS_MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) { projectiles[i] = {x, y, true}; return; }
        }
    }

    void spawnEnemy() {
        for (int i = 0; i < SS_MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                EnemyType t;
                int r = random(100);
                if (wave >= 5 && wave % 5 == 0) t = HEAVY;
                else if (wave >= 3 && wave % 3 == 0) t = (r < 80) ? FAST : HEAVY;
                else t = (r < 60) ? BASIC : (r < 85) ? FAST : HEAVY;

                float hm = 1.0f + (wave / 3) * 0.5f;
                int hp;
                switch (t) { case BASIC: hp=(int)(2*hm); break; case FAST: hp=(int)(1*hm); break; case HEAVY: hp=(int)(5*hm); break; }
                enemies[i] = { (float)random(20, SS_W-20), (float)random(-40,-10),
                    (random(2)==0?1.0f:-1.0f)*(t==FAST?2.0f:1.0f), hp, t, true, millis(), 0 };
                return;
            }
        }
    }

    void spawnEBullet(float x, float y, float vx, float vy) {
        for (int i = 0; i < SS_MAX_ENEMY_BULLETS; i++) {
            if (!eBullets[i].active) { eBullets[i] = {x, y, vx, vy, true}; return; }
        }
    }

    void spawnExplosion(float x, float y) {
        for (int i = 0; i < SS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) { explosions[i] = {x, y, 0, true}; return; }
        }
    }

    // --- Update ---
    void updatePlayer(SensorData &input) {
        float targetX = SS_W / 2.0f + input.roll * -7.0f;
        float targetY = 180.0f - input.pitch * 2.0f;
        playerX += (targetX - playerX) * 0.15f;
        playerY += (targetY - playerY) * 0.15f;
        playerX = constrain(playerX, 10, SS_W - 10);
        playerY = constrain(playerY, 80, SS_H - 10);

        if (input.fireLeftPressed())  { spawnProjectile(playerX - 8, playerY - 5); fireLeftAnim = millis(); SFX::shoot(); }
        if (input.fireRightPressed()) { spawnProjectile(playerX + 8, playerY - 5); fireRightAnim = millis(); SFX::shoot(); }
    }

    void updateStars() {
        for (int i = 0; i < SS_MAX_STARS; i++) {
            stars[i].y += stars[i].speed * dt * 60;
            if (stars[i].y > SS_H) { stars[i].y = 0; stars[i].x = random(SS_W); stars[i].speed = random(20,80)/10.0f; }
        }
    }

    void updateProjectiles() {
        for (int i = 0; i < SS_MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) continue;
            projectiles[i].y -= 6.0f * dt * 60;
            if (projectiles[i].y < -5) { projectiles[i].active = false; continue; }
            for (int j = 0; j < SS_MAX_ENEMIES; j++) {
                if (!enemies[j].active) continue;
                float dx = projectiles[i].x - enemies[j].x;
                float dy = projectiles[i].y - enemies[j].y;
                float hitR = (enemies[j].type == HEAVY) ? 14 : 10;
                if (dx*dx + dy*dy < hitR*hitR) {
                    projectiles[i].active = false;
                    enemies[j].hp--;
                    enemies[j].hitTime = millis();
                    if (enemies[j].hp <= 0) {
                        enemies[j].active = false;
                        spawnExplosion(enemies[j].x, enemies[j].y);
                        SFX::explode();
                        switch (enemies[j].type) { case BASIC: score+=100; break; case FAST: score+=150; break; case HEAVY: score+=300; break; }
                    } else { SFX::hit(); }
                    break;
                }
            }
        }
    }

    void updateEnemies() {
        unsigned long now = millis();
        for (int i = 0; i < SS_MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            if (enemies[i].y < 40) { enemies[i].y += 1.5f * dt * 60; }
            else { enemies[i].x += enemies[i].vx * dt * 60; if (enemies[i].x < 10 || enemies[i].x > SS_W-10) enemies[i].vx = -enemies[i].vx; }

            unsigned long cd;
            switch (enemies[i].type) { case BASIC: cd=2000; break; case FAST: cd=1500; break; case HEAVY: cd=1800; break; }
            if (enemies[i].y > 10 && now - enemies[i].lastShot > cd) {
                enemies[i].lastShot = now;
                if (enemies[i].type == HEAVY) {
                    spawnEBullet(enemies[i].x, enemies[i].y+5, -1.0f, 2.5f);
                    spawnEBullet(enemies[i].x, enemies[i].y+5,  0.0f, 3.0f);
                    spawnEBullet(enemies[i].x, enemies[i].y+5,  1.0f, 2.5f);
                } else if (enemies[i].type == FAST) {
                    float dx = playerX - enemies[i].x, dy = playerY - enemies[i].y;
                    float len = sqrtf(dx*dx+dy*dy);
                    if (len > 0) spawnEBullet(enemies[i].x, enemies[i].y+5, dx/len*2.5f, dy/len*2.5f);
                } else {
                    spawnEBullet(enemies[i].x, enemies[i].y+5, 0, 3.0f);
                }
            }
            if (enemies[i].y > SS_H + 20) enemies[i].active = false;
        }
    }

    void updateEBullets() {
        for (int i = 0; i < SS_MAX_ENEMY_BULLETS; i++) {
            if (!eBullets[i].active) continue;
            eBullets[i].x += eBullets[i].vx * dt * 60;
            eBullets[i].y += eBullets[i].vy * dt * 60;
            if (eBullets[i].y > SS_H+5 || eBullets[i].y < -5 || eBullets[i].x < -5 || eBullets[i].x > SS_W+5)
                { eBullets[i].active = false; continue; }
            float dx = eBullets[i].x - playerX, dy = eBullets[i].y - playerY;
            if (dx*dx + dy*dy < 100) { eBullets[i].active = false; playerHP -= 10; playerHitTime = millis(); if (playerHP <= 0) { playerHP = 0; deathTime = millis(); isGameOver = true; SFX::die(); } else { SFX::hit(); } }
        }
    }

    void updateExplosions() {
        for (int i = 0; i < SS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) continue;
            explosions[i].frame++;
            if (explosions[i].frame > 8) explosions[i].active = false;
        }
    }

    void updateWaves() {
        unsigned long now = millis();
        if (!waveActive && now >= nextWaveTime) {
            wave++; waveActive = true; enemiesSpawned = 0; SFX::wave();
            enemiesToSpawn = 5 + (wave-1)*2; nextSpawnTime = now + 500; waveAnnounceEnd = now + 2000;
        }
        if (waveActive && enemiesSpawned < enemiesToSpawn && now >= nextSpawnTime) {
            spawnEnemy(); enemiesSpawned++; nextSpawnTime = now + max(500, 1500 - wave*50);
        }
        if (waveActive && enemiesSpawned >= enemiesToSpawn) {
            bool allDead = true;
            for (int i = 0; i < SS_MAX_ENEMIES; i++) if (enemies[i].active) { allDead = false; break; }
            if (allDead) { waveActive = false; nextWaveTime = now + 3000; waveAnnounceEnd = 0; }
        }
    }

    // --- Draw ---
    void drawStars(TFT_eSprite &fb) {
        for (int i = 0; i < SS_MAX_STARS; i++) {
            uint8_t b = (uint8_t)(stars[i].speed * 30);
            fb.drawPixel(stars[i].x, stars[i].y, fb.color565(b, b, b));
        }
    }

    void drawPlayer(TFT_eSprite &fb) {
        int px = (int)playerX, py = (int)playerY;
        bool flashing = (millis() - playerHitTime) < 150;
        uint16_t bodyCol = flashing ? TFT_RED : TFT_WHITE;
        fb.fillRoundRect(px-10, py-12, 20, 14, 5, bodyCol);
        fb.fillCircle(px-4, py-6, 3, TFT_BLACK);
        fb.fillCircle(px+4, py-6, 2, TFT_BLACK);
        fb.drawLine(px-1, py-6, px+2, py-6, TFT_BLACK);

        unsigned long now = millis();
        bool lF = (now - fireLeftAnim) < SS_FIRE_ANIM_DURATION;
        bool rF = (now - fireRightAnim) < SS_FIRE_ANIM_DURATION;
        int lO = lF ? 3 : 0, rO = rF ? 3 : 0;
        fb.drawLine(px-7, py-12, px-10, py-20-lO, fb.color565(200,200,210));
        fb.fillCircle(px-10, py-21-lO, 2, lF ? TFT_YELLOW : TFT_RED);
        if (lF) fb.fillCircle(px-10, py-21-lO, 4, fb.color565(255,200,0));
        fb.drawLine(px+7, py-12, px+10, py-20-rO, fb.color565(200,200,210));
        fb.fillCircle(px+10, py-21-rO, 2, rF ? TFT_YELLOW : TFT_RED);
        if (rF) fb.fillCircle(px+10, py-21-rO, 4, fb.color565(255,200,0));
    }

    void drawProjectiles(TFT_eSprite &fb) {
        for (int i = 0; i < SS_MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) continue;
            fb.fillRect(projectiles[i].x-1, projectiles[i].y-3, 3, 6, TFT_GREEN);
        }
    }

    void drawEnemies(TFT_eSprite &fb) {
        for (int i = 0; i < SS_MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            int ex = (int)enemies[i].x, ey = (int)enemies[i].y;
            bool flashing = (millis() - enemies[i].hitTime) < 120;
            uint16_t col = flashing ? TFT_WHITE : enemyColor(enemies[i].type);
            int sz = (enemies[i].type == HEAVY) ? 10 : 7;
            fb.fillTriangle(ex, ey+sz, ex-sz, ey-sz/2, ex+sz, ey-sz/2, col);
            fb.fillCircle(ex, ey, 3, flashing ? TFT_RED : TFT_WHITE);
        }
    }

    void drawEBullets(TFT_eSprite &fb) {
        for (int i = 0; i < SS_MAX_ENEMY_BULLETS; i++) {
            if (!eBullets[i].active) continue;
            fb.fillCircle((int)eBullets[i].x, (int)eBullets[i].y, 2, TFT_RED);
        }
    }

    void drawExplosions(TFT_eSprite &fb) {
        for (int i = 0; i < SS_MAX_EXPLOSIONS; i++) {
            if (!explosions[i].active) continue;
            int r = explosions[i].frame * 2 + 2;
            uint8_t a = 255 - explosions[i].frame * 30;
            fb.drawCircle((int)explosions[i].x, (int)explosions[i].y, r, fb.color565(a, a/2, 0));
            fb.drawCircle((int)explosions[i].x, (int)explosions[i].y, r/2, TFT_YELLOW);
        }
    }

    void drawHUD(TFT_eSprite &fb) {
        fb.drawRect(5, 5, 102, 10, TFT_WHITE);
        uint16_t hpCol = (playerHP > 50) ? TFT_GREEN : (playerHP > 25) ? TFT_YELLOW : TFT_RED;
        fb.fillRect(6, 6, playerHP, 8, hpCol);
        fb.setTextColor(TFT_WHITE); fb.setTextSize(1);
        fb.setCursor(SS_W - 80, 7); fb.printf("SCR:%d", score);
        fb.setCursor(140, 7); fb.printf("W:%d", wave);
        if (waveAnnounceEnd > 0 && millis() < waveAnnounceEnd) {
            fb.setTextColor(TFT_YELLOW); fb.setTextSize(2);
            char ws[16]; sprintf(ws, "WAVE %d", wave);
            fb.drawCentreString(ws, SS_W/2, SS_H/2 - 20, 1);
        }
    }

    void drawGameOver(TFT_eSprite &fb) {
        fb.setTextColor(TFT_RED); fb.setTextSize(3);
        fb.drawCentreString("GAME OVER", SS_W/2, 70, 1);
        fb.setTextColor(TFT_CYAN); fb.setTextSize(2);
        char s[32]; sprintf(s, "SCORE: %d", score);
        fb.drawCentreString(s, SS_W/2, 120, 1);
        fb.setTextColor(TFT_GREEN); fb.setTextSize(1);
        fb.drawCentreString("FIRE TO RESTART", SS_W/2, 160, 1);
        fb.setTextColor(0x7BEF); fb.setTextSize(1);
        fb.drawCentreString("touch bas = menu", SS_W/2, 220, 1);
    }
};
