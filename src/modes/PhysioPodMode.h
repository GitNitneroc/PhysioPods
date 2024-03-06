#ifndef PhysioPodMode_h
#define PhysioPodMode_h
#include <Arduino.h>
#include "scoreStorage.h"

/*
    * This is the base class for all the modes that the PhysioPod can be in.
    * It is used to define the basic functions that all the modes should have.
*/
class PhysioPodMode {
public:
    virtual void stop() {}

    virtual ~PhysioPodMode() {}

    virtual void start() {}
    virtual void update() {}
    /*
        * This function is called when the mode is reset
        * It is used to reset the score and state of the mode
    */
    virtual void reset() {}
    virtual String* returnScore() { return nullptr; }
    virtual void onPodPressed(uint8_t id) {}
};

#endif