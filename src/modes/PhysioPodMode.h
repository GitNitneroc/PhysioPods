#ifndef PhysioPodMode_h
#define PhysioPodMode_h
#include <Arduino.h>
#include "scoreStorage.h"

/*
    * This is the base class for all the modes that the PhysioPod can be in.
    * It is used to define the basic functions that all the modes should have.
*/
class PhysioPodMode {
protected:
    bool running = false;

public:
    bool isRunning() { return running; }
    
    virtual const char* getName() = 0;
    virtual void stop();

    virtual ~PhysioPodMode() {}

    virtual void start();
    virtual void update() {}
    /*
        * This function is called when the mode is reset, start should be callable after this function
    */
    virtual void reset() {}
    virtual String* returnScore() { return nullptr; }
    virtual void onPodPressed(uint8_t id) {}

    static PhysioPodMode* currentMode;
};

#endif