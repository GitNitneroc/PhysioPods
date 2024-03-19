#ifndef PhysioPodMode_h
#define PhysioPodMode_h
#include <Arduino.h>
#include "scoreStorage.h"
#include "Messages.h"
#include "esp_now.h"

/*
    * This is the base class for all the modes that the PhysioPod can be in.
    * It is used to define the basic functions that all the modes should have.
*/
class PhysioPodMode {
protected:
    bool running = false;
    static void OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len);

public:
    bool isRunning() { return running; }
    
    virtual const char* getName() = 0;
    virtual void stop(); //this function should be implemented by the mode, and call PhysioPodMode::stop(). NB : This will call the reset function

    virtual ~PhysioPodMode() {}

    virtual void start(); //this should be implemeted by the mode, and call PhysioPodMode::start()
    virtual void update() {} //this function should be called every loop

    virtual void reset() {}// this should be implemented by the mode, making it clean for a new call to start()
    virtual String* returnScore() { return nullptr; } //this function should return a JSON string with the current score
    virtual void onPodPressed(uint8_t id) {} //this function is called when a pod is pressed

    static PhysioPodMode* currentMode;
};

#endif