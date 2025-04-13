#ifndef PhysioPod_h
#define PhysioPod_h

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <FastLED.h>

//Our control
#include "controls/ButtonControl.h"
#include "controls/CapacitiveTouchControl.h"
#include "controls/ProximityControl.h"
#include "controls/PiezoControl.h"

#include "modes/PhysioPodMode.h"
#include "LightModes.h"

class PhysioPod {
protected :
    uint8_t id;
    uint16_t sessionId = 0;
    static PhysioPod* instance;


public :
    //WIFI settings :
    static constexpr const char* password = "0123456789";

    //light settings : 
    //this stays static, as the PhysioPod is not created at the start, it's only initialised when the pod is created as client or server
    static CRGB leds[NUM_LEDS]; 
    static LightMode lightMode;
    static bool lightState;
    static CRGB lightColor;
    static uint16_t lightModeSpecificData; //this is used to store the mode specific data, like the speed of the light
    static bool lightChanged; //this is used by SetOwnLightState to signify something in the light has changed

    /*
        * This function is called to search for other PhysioPods
        * It will scan for WiFi networks and look for the PhysioPod network
        * It will return true if it found a PhysioPod network, and false otherwise
    */
    static bool searchOtherPhysioWiFi();

    static String getSSIDFromPreferences();
    
    static PhysioPod* getInstance(){
        return instance;
    }

    virtual void updatePod() = 0;

    static void initLEDs();
    void CreateControl(); //This will create the control for the pod, but it will not call initialize on it

    PhysioPodControl* control = nullptr;

    void setId(uint8_t id) {
        this->id = id;
    }
    uint8_t getId() {
        return id;
    }

    uint16_t getSessionId();

    /* This is used to set this pod light on or off*/
    static void setOwnLightState(bool state, CRGB color = CRGB::White, LightMode mode = LightMode::SIMPLE, uint16_t modeSpecific = 0);

    /* This will turn on/off and animate lights */
    static void manageOwnLight(void *param);
};

#endif