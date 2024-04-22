#ifndef PhysioPodMode_h
#define PhysioPodMode_h
#include <Arduino.h>
#include "scoreStorage.h"
#include "Messages.h"
#include "esp_now.h"
#include "controls/PhysioPodControl.h"

//TODO : modify how the modes store their parameters : they are basically here two times now, once in the static parameter struct, and once in the instance of the mode

/*
    * This is the base class for all the modes that the PhysioPod can be in.
    * It is used to define the basic functions that all the modes should have.
*/
class PhysioPodMode {
protected:
    bool running = false;
    //static void OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len);
    static PhysioPodControl *control;

public:
    bool isRunning() { return running; }

    static PhysioPodMode* (*modeConstructor)(); //Whenever this is not nullptr, it will be called to create a new instance of the mode
    
    static void setControl(PhysioPodControl *control);
    
    virtual const char* getName() { return "PhysioPodMode";} //this should nether be called, this class should actually be abstract
    virtual void stop(); //this function should be implemented by the mode, and call PhysioPodMode::stop(). NB : This will call the reset function

    virtual ~PhysioPodMode() {}
/* 
    static bool testRequestParameters(AsyncWebServerRequest *request){} //this function should be implemented by the mode, and test the parameters given in the request; if valid, it should return true, and store the params, to be able to create a new PhysioPodMode using generateMode()
    static void generateMode() {} //this function should be implemented by the mode, and create a new instance of the mode with the parameters stored in the class
 */
    virtual void start(); //this should be implemeted by the mode, and call PhysioPodMode::start()
    virtual void update() {} //this function should be called every loop

    virtual void reset() {}// this should be implemented by the mode, making it clean for a new call to start()
    virtual String* returnScore() { return nullptr; } //this function should return a JSON string with the current score
    virtual void onPodPressed(uint8_t id) {} //this function is called when a pod is pressed

    static PhysioPodMode* currentMode;
};

#endif