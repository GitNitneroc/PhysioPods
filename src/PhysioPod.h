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

#include "modes/PhysioPodMode.h"

class PhysioPod {
protected :
    uint8_t id;
    uint16_t sessionId = 0;
    static PhysioPod* instance;
    #ifdef USE_NEOPIXEL
    static CRGB leds[NUM_LEDS];
    #endif

public :
    //WIFI settings :
    static constexpr const char* ssid = "PhysioPods"; //having this as a static member rather than a define allows us to change it at runtime if needed someday
    static constexpr const char* password = "0123456789";
    /*
        * This function is called to search for other PhysioPods
        * It will scan for WiFi networks and look for the PhysioPod network
        * It will return true if it found a PhysioPod network, and false otherwise
    */
    static bool searchOtherPhysioWiFi();
    
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
    static void setOwnLightState(bool state, uint8_t r=0, uint8_t g=0, uint8_t b=0) ;
};

#endif