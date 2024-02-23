#ifndef PhysioPodMode_h
#define PhysioPodMode_h
#include <Arduino.h>
#include "scoreStorage.h"

class PhysioPodMode {
public:
    //virtual void initialize() = 0;
    virtual void stop() = 0;

    virtual ~PhysioPodMode() {}

    virtual void start() = 0;
    virtual void update() = 0;
    /*
        * Call this to reset the mode
        * This will reset the score
    */
    virtual void reset() = 0;
    virtual String* returnScore() = 0;
    static ScoreStorage* scoreStorage;
};

#endif