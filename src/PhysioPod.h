#ifndef PhysioPod_h
#define PhysioPod_h

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include "pins.h"
#include "controls/ButtonControl.h"

//TODO : il faudrait pouvoir définir le type de controle qu'on veut pour le pod, depuis le main.cpp et pas en rentrant dans le code comme ça
class PhysioPod {
protected :
    uint8_t id;

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

    virtual void updatePod() = 0;

    PhysioPodControl* control = nullptr;

    void setId(uint8_t id) {
        this->id = id;
    }
    uint8_t getId() {
        return id;
    }

    /* This is used to set this pod light on or off*/
    static void setOwnLightState(bool state) ;
    static void setOwnLightState(bool state, uint8_t r, uint8_t g, uint8_t b) ;
};

#endif