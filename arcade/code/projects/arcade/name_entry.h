#pragma once
#include <TFT_eSPI.h>
#include "game.h"
#include "scores.h"
#include "sound.h"

#define NE_W 320
#define NE_H 240
#define NE_ROLL_THRESHOLD 12.0f

class NameEntry {
public:
    char letters[NAME_LEN + 1];
    int cursor; // 0-3 = letters, 4 = OK
    bool active;
    int gameIndex;
    int finalScore;
    bool prevRollLeft = false;
    bool prevRollRight = false;

    void start(int gi, int score, const char* defaultName) {
        active = true;
        gameIndex = gi;
        finalScore = score;
        cursor = 0;
        prevRollLeft = false;
        prevRollRight = false;
        strncpy(letters, defaultName, NAME_LEN);
        letters[NAME_LEN] = '\0';
        for (int i = 0; i < NAME_LEN; i++) {
            if (letters[i] < 'A' || letters[i] > 'Z') letters[i] = 'A';
        }
    }

    // Returns true when done
    bool update(SensorData &input) {
        if (!active) return false;

        // Roll left = advance cursor
        // Roll right = go back
        bool rollLeft = input.roll < -NE_ROLL_THRESHOLD;
        bool rollRight = input.roll > NE_ROLL_THRESHOLD;

        if (rollLeft && !prevRollLeft) {
            if (cursor < NAME_LEN) {
                cursor++;
                SFX::select();
            }
        }
        if (rollRight && !prevRollRight) {
            if (cursor > 0) {
                cursor--;
                SFX::select();
            }
        }
        prevRollLeft = rollLeft;
        prevRollRight = rollRight;

        if (cursor < NAME_LEN) {
            // On a letter: antennas cycle letter
            if (input.fireRightPressed()) {
                letters[cursor]++;
                if (letters[cursor] > 'Z') letters[cursor] = 'A';
                SFX::hit();
            }
            if (input.fireLeftPressed()) {
                letters[cursor]--;
                if (letters[cursor] < 'A') letters[cursor] = 'Z';
                SFX::hit();
            }
        } else {
            // On OK: either antenna validates
            if (input.fireLeftPressed() || input.fireRightPressed()) {
                active = false;
                SFX::score();
                return true;
            }
        }

        return false;
    }

    void draw(TFT_eSprite &fb) {
        if (!active) return;

        // Dark overlay on bottom half
        int overlayY = 130;
        fb.fillRect(0, overlayY, NE_W, NE_H - overlayY, fb.color565(0, 0, 0));
        fb.drawFastHLine(0, overlayY, NE_W, fb.color565(80, 80, 80));

        // "NEW BEST!" banner
        fb.setTextColor(TFT_YELLOW);
        fb.setTextSize(1);
        fb.drawCentreString("NEW BEST!", NE_W / 2, overlayY + 5, 2);

        // Draw 4 letter boxes + OK button
        int boxW = 30;
        int gap = 8;
        int valW = 50;
        int totalW = NAME_LEN * boxW + (NAME_LEN - 1) * gap + gap + valW;
        int startX = (NE_W - totalW) / 2;
        int boxY = overlayY + 28;
        int boxH = 36;

        for (int i = 0; i < NAME_LEN; i++) {
            int bx = startX + i * (boxW + gap);
            bool selected = (i == cursor);

            uint16_t boxCol = selected ? TFT_YELLOW : fb.color565(60, 60, 80);
            fb.drawRoundRect(bx, boxY, boxW, boxH, 4, boxCol);
            if (selected) {
                fb.drawRoundRect(bx - 1, boxY - 1, boxW + 2, boxH + 2, 5, boxCol);
            }

            fb.setTextColor(selected ? TFT_YELLOW : TFT_WHITE);
            fb.setTextSize(2);
            char ch[2] = { letters[i], '\0' };
            fb.drawCentreString(ch, bx + boxW / 2, boxY + 8, 1);

            // Arrows on selected letter
            if (selected) {
                int cx = bx + boxW / 2;
                fb.fillTriangle(cx, boxY - 7, cx - 4, boxY - 3, cx + 4, boxY - 3, TFT_YELLOW);
                fb.fillTriangle(cx, boxY + boxH + 7, cx - 4, boxY + boxH + 3, cx + 4, boxY + boxH + 3, TFT_YELLOW);
            }
        }

        // OK button
        int valX = startX + NAME_LEN * (boxW + gap);
        bool valSelected = (cursor == NAME_LEN);
        uint16_t valCol = valSelected ? TFT_GREEN : fb.color565(60, 60, 80);
        fb.fillRoundRect(valX, boxY, valW, boxH, 4, valSelected ? fb.color565(0, 60, 0) : fb.color565(20, 20, 30));
        fb.drawRoundRect(valX, boxY, valW, boxH, 4, valCol);
        if (valSelected) {
            fb.drawRoundRect(valX - 1, boxY - 1, valW + 2, boxH + 2, 5, valCol);
        }
        fb.setTextColor(valCol);
        fb.setTextSize(1);
        fb.drawCentreString("OK", valX + valW / 2, boxY + 10, 2);

        // Instructions
        fb.setTextSize(1);
        fb.setTextColor(fb.color565(120, 120, 120));
        fb.drawCentreString("Antennas=letter/OK  Roll=move", NE_W / 2, overlayY + 75, 1);
    }
};
