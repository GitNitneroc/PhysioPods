#include <Arduino.h>
#include "isDebug.h"
#ifndef scoreStorage_h
#define scoreStorage_h

/* class ScoreStorage
{
private:
    String* scores[3];

public:
    ScoreStorage();
    ~ScoreStorage();
    String getAllJSON();
    String getLastJSON();
    void addScore(String* score);
    void updateScore(String* score);
}; */

namespace ScoreStorage
{
    void init();
    void updateScore(String* score);
    void addScore(String* score);
    String getAllJSON();
    String getLastJSON();
}

#endif // ScoreStorage_h