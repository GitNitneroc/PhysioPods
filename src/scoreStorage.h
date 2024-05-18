#include <Arduino.h>
#ifndef scoreStorage_h
#define scoreStorage_h

namespace ScoreStorage
{
    extern String* scores[3];

    void init();
    void updateScore(String* score);
    void addScore(String* score);
    String getAllJSON();
    String getLastJSON();
}

#endif // ScoreStorage_h