#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "game.h"

#define SERIAL_BAUD 2000000

// Touch XPT2046 sur VSPI
#define XPT_CLK  25
#define XPT_MISO 39
#define XPT_MOSI 32
#define XPT_CS   33
#define XPT_IRQ  36

class Input {
public:
    SensorData sensor;
    TouchData touch;

    void begin() {
        Serial.begin(SERIAL_BAUD);
        touchSPI.begin(XPT_CLK, XPT_MISO, XPT_MOSI, XPT_CS);
        ts.begin(touchSPI);
        ts.setRotation(1);
    }

    void update() {
        readSerial();
        readTouch();
    }

private:
    String buf;
    SPIClass touchSPI = SPIClass(VSPI);
    XPT2046_Touchscreen ts = XPT2046_Touchscreen(XPT_CS, XPT_IRQ);
    unsigned long lastTouchTime = 0;

    void readSerial() {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                JsonDocument doc;
                if (deserializeJson(doc, buf) == DeserializationError::Ok) {
                    sensor.roll  = doc["roll"]  | 0.0f;
                    sensor.pitch = doc["pitch"] | 0.0f;
                    sensor.yaw   = doc["yaw"]   | 0.0f;
                    sensor.fireLeft  = doc["fire_left"]  | false;
                    sensor.fireRight = doc["fire_right"] | false;
                }
                buf = "";
            } else {
                buf += c;
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
        } else {
            if (millis() - lastTouchTime > 100) {
                touch.touched = false;
            }
        }
    }
};
