#pragma once
#include <TFT_eSPI.h>
#include "game.h"

#define MAX_GAMES 8
#define ICON_W 80
#define ICON_H 70
#define COLS 3
#define MARGIN_X 20
#define MARGIN_Y 45
#define GAP_X 15
#define GAP_Y 12

// Icon descriptor
struct GameEntry {
    const char *name;
    uint16_t iconColor;
    // Simple icon symbol drawn in the box
    void (*drawIcon)(TFT_eSprite &fb, int cx, int cy);
    Game *(*create)();
};

// --- Icon draw functions ---

// Spaceship icon: Reachy Mini with antennas + stars
static void iconSpaceship(TFT_eSprite &fb, int cx, int cy) {
    // Stars
    fb.drawPixel(cx - 16, cy - 10, TFT_WHITE);
    fb.drawPixel(cx + 18, cy - 6, TFT_WHITE);
    fb.drawPixel(cx - 12, cy + 8, TFT_WHITE);
    fb.drawPixel(cx + 14, cy + 12, TFT_WHITE);
    fb.drawPixel(cx + 8, cy - 14, TFT_WHITE);
    fb.drawPixel(cx - 18, cy + 2, TFT_WHITE);
    // Reachy Mini head
    fb.fillRoundRect(cx - 6, cy - 8, 12, 8, 3, TFT_WHITE);
    fb.fillCircle(cx - 2, cy - 5, 1, TFT_BLACK);
    fb.fillCircle(cx + 2, cy - 5, 1, TFT_BLACK);
    fb.drawFastHLine(cx - 1, cy - 5, 2, TFT_BLACK);
    // Body
    fb.fillRoundRect(cx - 5, cy, 10, 7, 2, TFT_WHITE);
    // Base
    fb.fillRect(cx - 4, cy + 7, 8, 2, fb.color565(50, 50, 50));
    // Antennas
    fb.drawLine(cx - 4, cy - 8, cx - 6, cy - 14, fb.color565(200, 200, 210));
    fb.fillCircle(cx - 6, cy - 14, 1, TFT_RED);
    fb.drawLine(cx + 4, cy - 8, cx + 6, cy - 14, fb.color565(200, 200, 210));
    fb.fillCircle(cx + 6, cy - 14, 1, TFT_RED);
    // Thruster flame
    fb.fillCircle(cx, cy + 11, 2, TFT_ORANGE);
    fb.fillCircle(cx, cy + 12, 1, TFT_YELLOW);
}

// Pong icon: paddle + ball
static void iconPong(TFT_eSprite &fb, int cx, int cy) {
    fb.fillRect(cx - 15, cy - 8, 4, 16, TFT_WHITE);
    fb.fillRect(cx + 11, cy - 8, 4, 16, TFT_WHITE);
    fb.fillCircle(cx, cy, 3, TFT_YELLOW);
    fb.drawRect(cx - 20, cy - 14, 40, 28, 0x528A);
}

// Breakout icon: bricks + paddle
static void iconBreakout(TFT_eSprite &fb, int cx, int cy) {
    uint16_t cols[] = {TFT_RED, TFT_YELLOW, TFT_GREEN};
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            fb.fillRect(cx - 18 + c * 10, cy - 14 + r * 6, 8, 4, cols[r]);
        }
    }
    fb.fillRect(cx - 8, cy + 10, 16, 3, TFT_WHITE);
    fb.fillCircle(cx, cy + 6, 2, TFT_CYAN);
}

// Dino run icon: Mini Duck profile running toward Reachy
static void iconDino(TFT_eSprite &fb, int cx, int cy) {
    uint16_t orange = fb.color565(220, 100, 30);
    uint16_t gray = fb.color565(140, 140, 140);
    uint16_t dg = fb.color565(50, 50, 50);
    // Mini Duck — side profile
    // Head wide flat
    fb.fillRect(cx - 16, cy - 12, 10, 5, TFT_WHITE);
    fb.drawFastHLine(cx - 16, cy - 10, 10, orange);
    fb.fillCircle(cx - 8, cy - 10, 1, dg); // eye
    // Neck
    fb.fillRect(cx - 13, cy - 7, 3, 2, dg);
    // Torso
    fb.fillRect(cx - 15, cy - 5, 8, 7, TFT_WHITE);
    fb.drawFastHLine(cx - 15, cy - 2, 8, orange);
    // Bird legs
    fb.drawLine(cx - 13, cy + 2, cx - 14, cy + 5, TFT_WHITE);
    fb.drawLine(cx - 10, cy + 2, cx - 9, cy + 5, TFT_WHITE);
    // Feet
    fb.fillTriangle(cx - 17, cy + 5, cx - 12, cy + 5, cx - 17, cy + 7, orange);
    fb.fillTriangle(cx - 11, cy + 5, cx - 6, cy + 5, cx - 11, cy + 7, orange);
    // Ground
    fb.drawFastHLine(cx - 20, cy + 7, 40, 0x528A);
    // Reachy Mini obstacle
    fb.fillRoundRect(cx + 8, cy + 1, 8, 6, 3, TFT_WHITE);
    fb.fillRoundRect(cx + 7, cy - 4, 10, 6, 3, TFT_WHITE);
    fb.fillCircle(cx + 10, cy - 2, 1, TFT_BLACK);
    fb.fillCircle(cx + 14, cy - 2, 1, TFT_BLACK);
    fb.fillRect(cx + 8, cy + 7, 8, 1, TFT_BLACK);
}

// Flappy icon: Reachy Mini with propellers between pipes
static void iconFlappy(TFT_eSprite &fb, int cx, int cy) {
    uint16_t pipeCol = fb.color565(40, 160, 50);
    // Left pipe
    fb.fillRect(cx - 18, cy - 14, 6, 12, pipeCol);
    fb.fillRect(cx - 18, cy + 4, 6, 10, pipeCol);
    // Right pipe
    fb.fillRect(cx + 12, cy - 14, 6, 8, pipeCol);
    fb.fillRect(cx + 12, cy + 0, 6, 14, pipeCol);
    // Reachy Mini
    fb.fillRoundRect(cx - 4, cy - 4, 8, 7, 3, TFT_WHITE);
    fb.fillRoundRect(cx - 5, cy - 9, 10, 6, 3, TFT_WHITE);
    fb.fillCircle(cx - 2, cy - 7, 1, TFT_BLACK);
    fb.fillCircle(cx + 2, cy - 7, 1, TFT_BLACK);
    // Propellers
    fb.drawFastHLine(cx - 7, cy - 11, 5, TFT_WHITE);
    fb.drawFastHLine(cx + 2, cy - 11, 5, TFT_WHITE);
}

// Boss fight icon: big Reachy vs small player
static void iconBoss(TFT_eSprite &fb, int cx, int cy) {
    // Big boss Reachy
    fb.fillRoundRect(cx - 10, cy - 12, 20, 12, 4, TFT_WHITE);
    fb.fillCircle(cx - 4, cy - 7, 2, TFT_RED);
    fb.fillCircle(cx + 4, cy - 7, 2, TFT_RED);
    fb.drawLine(cx - 1, cy - 7, cx + 1, cy - 7, TFT_RED);
    fb.fillRoundRect(cx - 8, cy - 2, 16, 10, 4, TFT_WHITE);
    // Aura
    fb.drawRoundRect(cx - 12, cy - 14, 24, 26, 5, TFT_MAGENTA);
    // Antenna tips
    fb.drawLine(cx - 6, cy - 12, cx - 7, cy - 16, fb.color565(180, 180, 180));
    fb.drawLine(cx + 6, cy - 12, cx + 7, cy - 16, fb.color565(180, 180, 180));
    fb.fillCircle(cx - 7, cy - 17, 1, TFT_MAGENTA);
    fb.fillCircle(cx + 7, cy - 17, 1, TFT_MAGENTA);
    // Bullets
    fb.fillCircle(cx - 6, cy + 12, 1, TFT_RED);
    fb.fillCircle(cx, cy + 14, 1, TFT_RED);
    fb.fillCircle(cx + 6, cy + 12, 1, TFT_RED);
}

// Rhythm icon: two lanes with notes
static void iconRhythm(TFT_eSprite &fb, int cx, int cy) {
    // Lanes
    fb.drawRect(cx - 14, cy - 14, 10, 28, fb.color565(40, 40, 60));
    fb.drawRect(cx + 4, cy - 14, 10, 28, fb.color565(40, 40, 60));
    // Notes
    fb.fillRoundRect(cx - 13, cy - 10, 8, 5, 2, TFT_CYAN);
    fb.fillRoundRect(cx + 5, cy - 2, 8, 5, 2, TFT_MAGENTA);
    fb.fillRoundRect(cx - 13, cy + 4, 8, 5, 2, TFT_CYAN);
    // Hit line
    fb.drawFastHLine(cx - 16, cy + 10, 32, TFT_WHITE);
}

// Placeholder icon
static void iconTodo(TFT_eSprite &fb, int cx, int cy) {
    fb.drawRect(cx - 10, cy - 10, 20, 20, 0x528A);
    fb.drawLine(cx - 6, cy - 6, cx + 6, cy + 6, 0x528A);
    fb.drawLine(cx + 6, cy - 6, cx - 6, cy + 6, 0x528A);
}

class Menu {
public:
    int numGames = 0;
    GameEntry entries[MAX_GAMES];
    int selected = -1;
    bool showLeaderboard = false;
    ScoreManager *scoreMgr = nullptr;
    bool musicMuted = false;
    bool sfxMuted = false;
    unsigned long menuEnterTime = 0;

    void addGame(const char *name, uint16_t color, void (*drawFn)(TFT_eSprite&, int, int), Game*(*createFn)()) {
        if (numGames >= MAX_GAMES) return;
        entries[numGames++] = {name, color, drawFn, createFn};
    }

    void draw(TFT_eSprite &fb) {
        if (showLeaderboard) {
            drawLeaderboard(fb);
            return;
        }

        // Title
        fb.setTextColor(TFT_WHITE);
        fb.setTextSize(2);
        fb.drawCentreString("REACHY ARCADE", 160, 10, 1);

        // Draw icons in grid
        for (int i = 0; i < numGames; i++) {
            int col = i % COLS;
            int row = i / COLS;
            int x = MARGIN_X + col * (ICON_W + GAP_X);
            int y = MARGIN_Y + row * (ICON_H + GAP_Y);

            // Box
            fb.drawRoundRect(x, y, ICON_W, ICON_H, 5, entries[i].iconColor);

            // Icon
            int cx = x + ICON_W / 2;
            int cy = y + ICON_H / 2 - 6;
            entries[i].drawIcon(fb, cx, cy);

            // Label
            fb.setTextColor(entries[i].iconColor);
            fb.setTextSize(1);
            fb.drawCentreString(entries[i].name, cx, y + ICON_H - 12, 1);
        }

        // Bottom bar buttons
        fb.setTextSize(1);

        // Leaderboard button — bottom left
        fb.drawRoundRect(5, 220, 65, 16, 4, TFT_YELLOW);
        fb.setTextColor(TFT_YELLOW);
        fb.drawCentreString("SCORES", 37, 223, 1);

        // Music mute button
        uint16_t musCol = musicMuted ? TFT_RED : TFT_GREEN;
        fb.drawRoundRect(78, 220, 75, 16, 4, musCol);
        fb.setTextColor(musCol);
        fb.drawCentreString(musicMuted ? "MUSIC OFF" : "MUSIC ON", 115, 223, 1);

        // SFX mute button
        uint16_t sfxCol = sfxMuted ? TFT_RED : TFT_GREEN;
        fb.drawRoundRect(161, 220, 65, 16, 4, sfxCol);
        fb.setTextColor(sfxCol);
        fb.drawCentreString(sfxMuted ? "SFX OFF" : "SFX ON", 193, 223, 1);
    }

    // Returns index of touched game, or -1
    int handleTouch(TouchData &touch) {
        if (!touch.touched) return -1;

        if (showLeaderboard) {
            // Back button — bottom right
            if (touch.x >= 230 && touch.y >= 220) {
                showLeaderboard = false;
            }
            return -1;
        }

        // Bottom buttons blocked for 1s after entering menu
        bool buttonsReady = (millis() - menuEnterTime > 1000);

        if (buttonsReady && touch.y >= 220) {
            // Leaderboard button
            if (touch.x >= 5 && touch.x <= 70) {
                if (scoreMgr) scoreMgr->load();
                showLeaderboard = true;
                return -1;
            }

            // Music mute button
            if (touch.x >= 78 && touch.x <= 153) {
                musicMuted = !musicMuted;
                SFX::toggleMusic();
                return -1;
            }

            // SFX mute button
            if (touch.x >= 161 && touch.x <= 226) {
                sfxMuted = !sfxMuted;
                SFX::toggleSfx();
                return -1;
            }
        }

        for (int i = 0; i < numGames; i++) {
            int col = i % COLS;
            int row = i / COLS;
            int x = MARGIN_X + col * (ICON_W + GAP_X);
            int y = MARGIN_Y + row * (ICON_H + GAP_Y);

            if (touch.x >= x && touch.x <= x + ICON_W &&
                touch.y >= y && touch.y <= y + ICON_H) {
                return i;
            }
        }
        return -1;
    }

private:
    void drawLeaderboard(TFT_eSprite &fb) {
        fb.setTextColor(TFT_YELLOW);
        fb.setTextSize(2);
        fb.drawCentreString("BEST SCORES", 160, 10, 1);

        fb.drawFastHLine(40, 30, 240, fb.color565(60, 60, 60));

        if (!scoreMgr) return;

        for (int i = 0; i < NUM_GAMES; i++) {
            int y = 42 + i * 28;
            uint16_t col = (i < numGames) ? entries[i].iconColor : TFT_WHITE;

            fb.setTextColor(col);
            fb.setTextSize(1);
            fb.drawString(gameNames[i], 20, y, 2);

            if (scoreMgr->bestScores[i] > 0) {
                // Name
                fb.setTextColor(TFT_CYAN);
                fb.drawString(scoreMgr->bestNames[i], 150, y, 2);

                // Score
                char buf[16];
                sprintf(buf, "%d", scoreMgr->bestScores[i]);
                fb.setTextColor(TFT_WHITE);
                fb.drawRightString(buf, 300, y, 2);
            } else {
                fb.setTextColor(fb.color565(80, 80, 80));
                fb.drawRightString("---", 300, y, 2);
            }
        }

        // Back button — bottom right
        fb.drawRoundRect(235, 220, 80, 16, 4, 0x7BEF);
        fb.setTextColor(0x7BEF);
        fb.setTextSize(1);
        fb.drawCentreString("RETOUR", 275, 223, 1);
    }
};
