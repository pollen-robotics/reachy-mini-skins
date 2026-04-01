#pragma once
#include <TFT_eSPI.h>
#include "scores.h"

// Sensor data from bridge
struct SensorData {
    // Calibrated values (used by games)
    float roll = 0, pitch = 0, yaw = 0;
    // Raw values from bridge
    float rawRoll = 0, rawPitch = 0, rawYaw = 0;
    // Offsets
    float rollOffset = 0, pitchOffset = 0, yawOffset = 0;

    bool fireLeft = false, fireRight = false;
    bool prevFireLeft = false, prevFireRight = false;

    bool fireLeftPressed()  { return fireLeft && !prevFireLeft; }
    bool fireRightPressed() { return fireRight && !prevFireRight; }

    void latch() {
        prevFireLeft = fireLeft;
        prevFireRight = fireRight;
    }

    void applyCalibration() {
        roll = rawRoll - rollOffset;
        pitch = rawPitch - pitchOffset;
        yaw = rawYaw - yawOffset;
    }

    void calibrate() {
        rollOffset = rawRoll;
        pitchOffset = rawPitch;
        yawOffset = rawYaw;
        roll = 0; pitch = 0; yaw = 0;
    }
};

// Touch data
struct TouchData {
    bool touched = false;
    int x = 0, y = 0;
    int rawX = 0, rawY = 0;
};

// Game interface — each game implements this
class Game {
public:
    virtual ~Game() {}
    virtual void setup(TFT_eSprite &fb) = 0;
    virtual void update(float dt, SensorData &input, TouchData &touch) = 0;
    virtual void draw(TFT_eSprite &fb) = 0;
    virtual bool wantsExit() { return false; }
    virtual int getScore() { return 0; }
    virtual void sendState() {} // Override to stream state to bridge
    // Safe restart — stores fb ref from setup for reuse
    TFT_eSprite *_fb = nullptr;
    bool pendingNameEntry = false;
    bool isGameOver = false;
    void restart() {
        if (_fb) setup(*_fb);
    }
    // Score tracking
    int gameIndex = -1;
    ScoreManager *scores = nullptr;
};
