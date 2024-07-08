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

    //TODO remove the USE_NEOPIXEL define
    #ifdef USE_NEOPIXEL
    static TaskHandle_t ledTask; //TODO : is this needed ?
    #endif

public :
    //WIFI settings :
    static constexpr const char* password = "0123456789";

    //light settings
    //TODO : plutôt que tout avoir en static, on peut utiliser le singleton instance
    static CRGB leds[NUM_LEDS];
    static LightMode lightMode;
    static bool lightState;
    static CRGB lightColor;
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
    static void setOwnLightState(bool state, CRGB color = CRGB::White, LightMode mode = LightMode::SIMPLE);

    /* This will turn on/off and animate lights */
    static void manageOwnLight(void *param);
};

#endif