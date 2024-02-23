#include <Arduino.h>
#include "isDebug.h"
#ifndef scoreStorage_h
#define scoreStorage_h

class ScoreStorage
{
private:
    String* scores[3];

public:
    ScoreStorage(/* args */);
    ~ScoreStorage();
    String getAllJSON();
    String getLastJSON();
    void addScore(String* score);
    void updateScore(String* score);
};

#endif // ScoreStorage_h