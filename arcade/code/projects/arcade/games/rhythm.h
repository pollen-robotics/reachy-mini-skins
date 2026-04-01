#pragma once
#include "../game.h"
#include "../sound.h"

#define RH_W 320
#define RH_H 240
#define RH_MAX_NOTES 32
#define RH_LANES 4
#define RH_HIT_Y 200
#define RH_HIT_WINDOW 18
#define RH_GOOD_WINDOW 30
#define RH_YAW_THRESHOLD 12.0f

// Music sync: 146 BPM, ~61.4s
#define RH_BPM 146
#define RH_BEAT_INTERVAL (60.0f / RH_BPM)  // ~0.411s
#define RH_SONG_DURATION 61.4f
#define RH_TOTAL_BEATS 149

// Note speed: how fast notes fall (pixels per frame at 60fps)
// Time for note to fall from top to hit zone = RH_HIT_Y / (noteSpeed * 60)
// We want ~1.5s travel time so player can react
#define RH_NOTE_SPEED 2.2f
#define RH_SPAWN_ADVANCE (RH_HIT_Y / (RH_NOTE_SPEED * 60.0f))  // seconds ahead to spawn

class RhythmGame : public Game {
public:
    struct Note { float y; int lane; bool active; bool hit; };
    Note notes[RH_MAX_NOTES];

    int score, combo, maxCombo;
    int hits, misses, totalNotes;
    bool exitRequested, dead, won;
    unsigned long endTime;
    unsigned long lastHitTime[RH_LANES];
    int lastHitQuality[RH_LANES];
    unsigned long qualityTime[RH_LANES];

    // Roll edge detection
    bool rollLeftActive, rollRightActive;
    bool prevRollLeft, prevRollRight;

    // Beat tracking
    float songTime;         // elapsed time in song
    int nextBeat;           // next beat index to schedule
    int beatLanes[RH_TOTAL_BEATS + 4]; // pre-generated lane for each beat (-1 = rest)
    int beatLanes2[RH_TOTAL_BEATS + 4]; // second lane for double notes (-1 = none)

    // Lane X positions
    int laneX[RH_LANES];

    void generateBeatMap() {
        // Pre-generate which lane each beat goes to
        // Patterns: 0=roll left, 1=fire left, 2=fire right, 3=roll right
        // Roll lanes are rarer, never both rolls at once
        int patterns[][2] = {
            {1, -1},  // fire left
            {2, -1},  // fire right
            {1, -1},  // fire left
            {1,  2},  // both fire
            {2, -1},  // fire right
            {1, -1},  // fire left
            {0, -1},  // roll left
            {2, -1},  // fire right
            {1, -1},  // fire left
            {2, -1},  // fire right
            {1,  2},  // both fire
            {1, -1},  // fire left
            {3, -1},  // roll right
            {2, -1},  // fire right
            {-1,-1},  // rest
            {1, -1},  // fire left
        };
        int numPatterns = 16;

        for (int i = 0; i < RH_TOTAL_BEATS + 4; i++) {
            // First 4 beats = rest (intro)
            if (i < 4) {
                beatLanes[i] = -1;
                beatLanes2[i] = -1;
                continue;
            }
            // Sometimes random
            if (random(100) < 20) {
                int r = random(100);
                if (r < 40) beatLanes[i] = 1;
                else if (r < 80) beatLanes[i] = 2;
                else if (r < 90) beatLanes[i] = 0;
                else beatLanes[i] = 3;
                beatLanes2[i] = -1;
            } else {
                int pi = (i - 4) % numPatterns;
                beatLanes[i] = patterns[pi][0];
                beatLanes2[i] = patterns[pi][1];
            }
            // Occasional rest beats (10%)
            if (random(100) < 10) {
                beatLanes[i] = -1;
                beatLanes2[i] = -1;
            }
        }
        // Count total notes for end stats
        totalNotes = 0;
        for (int i = 0; i < RH_TOTAL_BEATS; i++) {
            if (beatLanes[i] >= 0) totalNotes++;
            if (beatLanes2[i] >= 0) totalNotes++;
        }
    }

    void reset() {
        exitRequested = false;
        dead = false;
        won = false;
        endTime = 0; isGameOver = false;
        score = 0; combo = 0; maxCombo = 0;
        hits = 0; misses = 0;
        songTime = -0.5f; // small delay before music starts
        nextBeat = 0;

        rollLeftActive = false; rollRightActive = false;
        prevRollLeft = false; prevRollRight = false;

        laneX[0] = RH_W / 2 - 90;
        laneX[1] = RH_W / 2 - 30;
        laneX[2] = RH_W / 2 + 30;
        laneX[3] = RH_W / 2 + 90;

        for (int i = 0; i < RH_MAX_NOTES; i++) notes[i].active = false;
        for (int i = 0; i < RH_LANES; i++) {
            lastHitTime[i] = 0;
            lastHitQuality[i] = 0;
            qualityTime[i] = 0;
        }

        generateBeatMap();
    }

    void setup(TFT_eSprite &fb) override { reset(); }

    void update(float dt, SensorData &input, TouchData &touch) override {
        if (dead || won) {
            if ((input.fireLeft || input.fireRight) && millis() - endTime > 1500) {
                restart();
            }
            if (touch.touched && touch.y > 200) exitRequested = true;
            return;
        }

        songTime += dt;

        // Spawn notes synced to beats
        // Spawn RH_SPAWN_ADVANCE seconds ahead so note arrives at hit zone on the beat
        while (nextBeat < RH_TOTAL_BEATS) {
            float beatTime = nextBeat * RH_BEAT_INTERVAL;
            float spawnTime = beatTime - RH_SPAWN_ADVANCE;
            if (songTime >= spawnTime) {
                if (beatLanes[nextBeat] >= 0) {
                    spawnSingleNote(beatLanes[nextBeat]);
                }
                if (beatLanes2[nextBeat] >= 0) {
                    spawnSingleNote(beatLanes2[nextBeat]);
                }
                nextBeat++;
            } else {
                break;
            }
        }

        // Move notes at constant speed
        for (int i = 0; i < RH_MAX_NOTES; i++) {
            if (!notes[i].active) continue;
            notes[i].y += RH_NOTE_SPEED * dt * 60;
            // Missed
            if (notes[i].y > RH_HIT_Y + RH_GOOD_WINDOW + 10 && !notes[i].hit) {
                notes[i].active = false;
                misses++;
                combo = 0;
                lastHitQuality[notes[i].lane] = 3;
                qualityTime[notes[i].lane] = millis();
            }
            // Off screen
            if (notes[i].y > RH_H + 10) notes[i].active = false;
        }

        // Roll edge detection
        rollLeftActive = (input.roll > RH_YAW_THRESHOLD);
        rollRightActive = (input.roll < -RH_YAW_THRESHOLD);
        bool rollLeftPressed = rollLeftActive && !prevRollLeft;
        bool rollRightPressed = rollRightActive && !prevRollRight;
        prevRollLeft = rollLeftActive;
        prevRollRight = rollRightActive;

        // Check input
        if (rollLeftPressed) checkHit(0);
        if (input.fireLeftPressed()) checkHit(1);
        if (input.fireRightPressed()) checkHit(2);
        if (rollRightPressed) checkHit(3);

        // Lose: too many misses
        if (misses >= 20) {
            dead = true;
            endTime = millis(); isGameOver = true;
            SFX::die();
            SFX::gameStop();
        }

        // Win: song finished and all notes resolved
        if (nextBeat >= RH_TOTAL_BEATS && songTime > RH_SONG_DURATION) {
            bool allDone = true;
            for (int i = 0; i < RH_MAX_NOTES; i++) {
                if (notes[i].active && !notes[i].hit) { allDone = false; break; }
            }
            if (allDone) {
                won = true;
                endTime = millis(); isGameOver = true;
                SFX::win();
                SFX::gameStop();
            }
        }
    }

    void draw(TFT_eSprite &fb) override {
        // Background lanes
        for (int i = 0; i < RH_LANES; i++) {
            int lx = laneX[i];
            fb.fillRect(lx - 18, 0, 36, RH_H, fb.color565(15, 15, 25));
            fb.drawRect(lx - 18, 0, 36, RH_H, fb.color565(30, 30, 50));
        }

        // Hit zone line
        fb.drawFastHLine(laneX[0] - 22, RH_HIT_Y, laneX[3] - laneX[0] + 44, fb.color565(60, 60, 80));

        // Hit targets
        for (int i = 0; i < RH_LANES; i++) {
            int lx = laneX[i];
            bool justHit = (millis() - lastHitTime[i]) < 120;
            uint16_t tCol = justHit ? TFT_WHITE : fb.color565(80, 80, 100);
            fb.drawRoundRect(lx - 14, RH_HIT_Y - 8, 28, 16, 4, tCol);
            if (justHit) fb.drawRoundRect(lx - 15, RH_HIT_Y - 9, 30, 18, 5, tCol);
        }

        // Lane labels
        fb.setTextSize(1);
        fb.setTextColor(fb.color565(100, 100, 120));
        fb.drawCentreString("<R", laneX[0], RH_HIT_Y + 14, 1);
        fb.drawCentreString("L", laneX[1], RH_HIT_Y + 14, 1);
        fb.drawCentreString("R", laneX[2], RH_HIT_Y + 14, 1);
        fb.drawCentreString("R>", laneX[3], RH_HIT_Y + 14, 1);

        // Notes
        for (int i = 0; i < RH_MAX_NOTES; i++) {
            if (!notes[i].active || notes[i].hit) continue;
            drawNote(fb, laneX[notes[i].lane], (int)notes[i].y, notes[i].lane);
        }

        // Quality feedback
        for (int i = 0; i < RH_LANES; i++) {
            if (millis() - qualityTime[i] < 400) {
                const char* txt;
                uint16_t col;
                switch (lastHitQuality[i]) {
                    case 1: txt = "PERFECT"; col = TFT_GREEN; break;
                    case 2: txt = "GOOD"; col = TFT_YELLOW; break;
                    default: txt = "MISS"; col = TFT_RED; break;
                }
                fb.setTextSize(1);
                fb.setTextColor(col);
                fb.drawCentreString(txt, laneX[i], RH_HIT_Y - 25, 1);
            }
        }

        // HUD
        fb.setTextColor(TFT_WHITE);
        fb.setTextSize(1);
        char s[32];
        sprintf(s, "%d", score);
        fb.drawString(s, 5, 5, 1);

        // Combo
        if (combo >= 3) {
            fb.setTextColor(TFT_YELLOW);
            fb.setTextSize(2);
            sprintf(s, "x%d", combo);
            fb.drawCentreString(s, RH_W / 2, 5, 1);
        }

        // Progress bar (song position)
        float progress = songTime / RH_SONG_DURATION;
        if (progress < 0) progress = 0;
        if (progress > 1) progress = 1;
        int barW = 60;
        int barX = RH_W - barW - 5;
        fb.drawRect(barX, 5, barW, 6, fb.color565(60, 60, 60));
        fb.fillRect(barX + 1, 6, (int)((barW - 2) * progress), 4, TFT_CYAN);

        // Miss counter
        fb.setTextColor(TFT_RED);
        fb.setTextSize(1);
        sprintf(s, "%d/20", misses);
        fb.drawString(s, barX - 30, 5, 1);

        if (dead) {
            fb.setTextColor(TFT_RED); fb.setTextSize(2);
            fb.drawCentreString("GAME OVER", RH_W / 2, 50, 1);
            fb.setTextColor(TFT_CYAN);
            sprintf(s, "SCORE: %d", score);
            fb.drawCentreString(s, RH_W / 2, 80, 1);
            fb.setTextColor(TFT_YELLOW); fb.setTextSize(1);
            sprintf(s, "HITS: %d/%d  COMBO: %d", hits, totalNotes, maxCombo);
            fb.drawCentreString(s, RH_W / 2, 105, 1);
            fb.setTextColor(TFT_GREEN);
            fb.drawCentreString("FIRE TO RESTART", RH_W / 2, 125, 1);
            fb.setTextColor(0x7BEF);
            fb.drawCentreString("touch bas = menu", RH_W / 2, 220, 1);
        }

        if (won) {
            fb.setTextColor(TFT_GREEN); fb.setTextSize(2);
            fb.drawCentreString("COMPLETE!", RH_W / 2, 45, 1);
            fb.setTextColor(TFT_CYAN);
            sprintf(s, "SCORE: %d", score);
            fb.drawCentreString(s, RH_W / 2, 75, 1);
            fb.setTextColor(TFT_YELLOW); fb.setTextSize(1);
            sprintf(s, "HITS: %d/%d  COMBO: %d", hits, totalNotes, maxCombo);
            fb.drawCentreString(s, RH_W / 2, 100, 1);

            // Rating
            float pct = (totalNotes > 0) ? (float)hits / totalNotes : 0;
            const char* rating;
            uint16_t rCol;
            if (pct > 0.95f) { rating = "S"; rCol = TFT_YELLOW; }
            else if (pct > 0.85f) { rating = "A"; rCol = TFT_GREEN; }
            else if (pct > 0.70f) { rating = "B"; rCol = TFT_CYAN; }
            else { rating = "C"; rCol = TFT_WHITE; }

            fb.setTextColor(rCol); fb.setTextSize(3);
            fb.drawCentreString(rating, RH_W / 2, 120, 1);

            fb.setTextColor(TFT_GREEN); fb.setTextSize(1);
            fb.drawCentreString("FIRE TO RESTART", RH_W / 2, 160, 1);
            fb.setTextColor(0x7BEF);
            fb.drawCentreString("touch bas = menu", RH_W / 2, 220, 1);
        }
    }

    bool wantsExit() override { return exitRequested; }
    int getScore() override { return score; }

    void sendState() override {
        unsigned long now = millis();
        char buf[512];
        int pos = 0;
        pos += snprintf(buf+pos, sizeof(buf)-pos,
            "{\"gs\":{\"sc\":%d,\"co\":%d,\"mi\":%d,\"dd\":%d,\"wn\":%d,\"st\":%.1f,\"tn\":%d,\"hi\":%d,\"mc\":%d,\"nt\":[",
            score, combo, misses, dead ? 1 : 0, won ? 1 : 0, songTime, totalNotes, hits, maxCombo);
        bool first = true;
        for (int i = 0; i < RH_MAX_NOTES; i++) {
            if (!notes[i].active || notes[i].hit) continue;
            if (!first) buf[pos++] = ',';
            pos += snprintf(buf+pos, sizeof(buf)-pos, "[%.0f,%d]", notes[i].y, notes[i].lane);
            first = false;
        }
        pos += snprintf(buf+pos, sizeof(buf)-pos, "],\"lh\":[%d,%d,%d,%d],\"lq\":[%d,%d,%d,%d],\"qt\":[%d,%d,%d,%d]}}",
            (now - lastHitTime[0]) < 120 ? 1 : 0,
            (now - lastHitTime[1]) < 120 ? 1 : 0,
            (now - lastHitTime[2]) < 120 ? 1 : 0,
            (now - lastHitTime[3]) < 120 ? 1 : 0,
            lastHitQuality[0], lastHitQuality[1], lastHitQuality[2], lastHitQuality[3],
            (now - qualityTime[0]) < 400 ? 1 : 0,
            (now - qualityTime[1]) < 400 ? 1 : 0,
            (now - qualityTime[2]) < 400 ? 1 : 0,
            (now - qualityTime[3]) < 400 ? 1 : 0);
        Serial.println(buf);
    }

private:
    void spawnSingleNote(int lane) {
        for (int i = 0; i < RH_MAX_NOTES; i++) {
            if (!notes[i].active) {
                notes[i] = { -10.0f, lane, true, false };
                return;
            }
        }
    }

    void checkHit(int lane) {
        lastHitTime[lane] = millis();

        float bestDist = 9999;
        int bestIdx = -1;
        for (int i = 0; i < RH_MAX_NOTES; i++) {
            if (!notes[i].active || notes[i].hit || notes[i].lane != lane) continue;
            float dist = fabsf(notes[i].y - RH_HIT_Y);
            if (dist < bestDist) { bestDist = dist; bestIdx = i; }
        }

        if (bestIdx >= 0 && bestDist < RH_GOOD_WINDOW) {
            notes[bestIdx].hit = true;
            notes[bestIdx].active = false;
            hits++;
            combo++;
            if (combo > maxCombo) maxCombo = combo;

            if (bestDist < RH_HIT_WINDOW) {
                score += 100 * (1 + combo / 5);
                lastHitQuality[lane] = 1;
                SFX::perfect();
            } else {
                score += 50 * (1 + combo / 5);
                lastHitQuality[lane] = 2;
                SFX::hit();
            }
            if (combo >= 5 && combo % 5 == 0) SFX::combo();
            qualityTime[lane] = millis();
        } else {
            combo = 0;
            lastHitQuality[lane] = 3;
            SFX::miss();
            qualityTime[lane] = millis();
        }
    }

    uint16_t laneColor(int lane) {
        switch (lane) {
            case 0: return TFT_GREEN;
            case 1: return TFT_CYAN;
            case 2: return TFT_MAGENTA;
            default: return TFT_YELLOW;
        }
    }

    void drawNote(TFT_eSprite &fb, int x, int y, int lane) {
        uint16_t col = laneColor(lane);
        fb.fillRoundRect(x - 10, y - 6, 20, 12, 4, col);
        fb.fillCircle(x - 3, y, 2, TFT_BLACK);
        fb.fillCircle(x + 3, y, 2, TFT_BLACK);
        fb.drawLine(x - 1, y, x + 1, y, TFT_BLACK);
    }
};
