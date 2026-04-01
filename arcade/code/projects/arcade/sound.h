#pragma once
#include <Arduino.h>

// Send sound events to bridge via Serial JSON
// The bridge plays the actual audio on the PC

namespace SFX {
    inline void send(const char* name) {
        Serial.print("{\"sfx\":\"");
        Serial.print(name);
        Serial.println("\"}");
    }

    // Game music control
    inline void gameStart(const char* name) {
        Serial.print("{\"game_start\":\"");
        Serial.print(name);
        Serial.println("\"}");
    }
    inline void gameStop() {
        Serial.println("{\"game_stop\":true}");
    }

    // Common sound events
    inline void shoot()     { send("shoot"); }
    inline void hit()       { send("hit"); }
    inline void explode()   { send("explode"); }
    inline void die()       { send("die"); }
    inline void score()     { send("score"); }
    inline void combo()     { send("combo"); }
    inline void miss()      { send("miss"); }
    inline void perfect()   { send("perfect"); }
    inline void jump()      { send("jump"); }
    inline void flap()      { send("flap"); }
    inline void bounce()    { send("bounce"); }
    inline void serve()     { send("serve"); }
    inline void win()       { send("win"); }
    inline void select()    { send("select"); }
    inline void wave()      { send("wave"); }
    inline void boss_hit()  { send("boss_hit"); }
    inline void phase()     { send("phase"); }

    // Mute toggles
    inline void toggleMusic() { Serial.println("{\"toggle_music\":true}"); }
    inline void toggleSfx()   { Serial.println("{\"toggle_sfx\":true}"); }
}
