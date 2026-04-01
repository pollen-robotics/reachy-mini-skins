#pragma once
#include <Preferences.h>

#define NUM_GAMES 6
#define NAME_LEN 4

static const char* gameKeys[NUM_GAMES] = {
    "spaceship", "pong", "duckrun", "flappy", "boss", "rhythm"
};
static const char* gameNames[NUM_GAMES] = {
    "Spaceship", "Pong", "Duck Run", "Flappy", "Boss", "Rhythm"
};

class ScoreManager {
public:
    int bestScores[NUM_GAMES];
    char bestNames[NUM_GAMES][NAME_LEN + 1]; // 4 chars + null
    char lastName[NAME_LEN + 1]; // last entered name (default for next entry)

    void load() {
        Preferences prefs;
        prefs.begin("arcade", true);
        for (int i = 0; i < NUM_GAMES; i++) {
            bestScores[i] = prefs.getInt(gameKeys[i], 0);
            String nameKey = String(gameKeys[i]) + "_n";
            String name = prefs.getString(nameKey.c_str(), "AAAA");
            strncpy(bestNames[i], name.c_str(), NAME_LEN);
            bestNames[i][NAME_LEN] = '\0';
        }
        String ln = prefs.getString("lastname", "AAAA");
        strncpy(lastName, ln.c_str(), NAME_LEN);
        lastName[NAME_LEN] = '\0';
        prefs.end();
    }

    bool isNewBest(int gameIndex, int score) {
        if (gameIndex < 0 || gameIndex >= NUM_GAMES) return false;
        return score > bestScores[gameIndex];
    }

    void save(int gameIndex, int score, const char* name) {
        if (gameIndex < 0 || gameIndex >= NUM_GAMES) return;
        bestScores[gameIndex] = score;
        strncpy(bestNames[gameIndex], name, NAME_LEN);
        bestNames[gameIndex][NAME_LEN] = '\0';
        strncpy(lastName, name, NAME_LEN);
        lastName[NAME_LEN] = '\0';

        Preferences prefs;
        prefs.begin("arcade", false);
        prefs.putInt(gameKeys[gameIndex], score);
        String nameKey = String(gameKeys[gameIndex]) + "_n";
        prefs.putString(nameKey.c_str(), name);
        prefs.putString("lastname", name);
        prefs.end();

        // Notify bridge so scores stay in sync
        sendScoreUpdate(gameIndex);
    }

    // Send a single score update to the bridge
    void sendScoreUpdate(int gameIndex) {
        if (gameIndex < 0 || gameIndex >= NUM_GAMES) return;
        Serial.print("{\"score_update\":{\"game\":");
        Serial.print(gameIndex);
        Serial.print(",\"score\":");
        Serial.print(bestScores[gameIndex]);
        Serial.print(",\"name\":\"");
        Serial.print(bestNames[gameIndex]);
        Serial.println("\"}}");
    }

    // Receive a score update from the bridge (no re-send to avoid loop)
    void receiveUpdate(int gameIndex, int score, const char* name) {
        if (gameIndex < 0 || gameIndex >= NUM_GAMES) return;
        bestScores[gameIndex] = score;
        strncpy(bestNames[gameIndex], name, NAME_LEN);
        bestNames[gameIndex][NAME_LEN] = '\0';

        Preferences prefs;
        prefs.begin("arcade", false);
        prefs.putInt(gameKeys[gameIndex], score);
        String nameKey = String(gameKeys[gameIndex]) + "_n";
        prefs.putString(nameKey.c_str(), name);
        prefs.end();
    }

    // Old submit for compatibility (restart without name entry)
    bool submit(int gameIndex, int score) {
        if (!isNewBest(gameIndex, score)) return false;
        save(gameIndex, score, lastName);
        return true;
    }
};
