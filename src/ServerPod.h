#include "PhysioPod.h"
#ifndef ServerPod_h
#define ServerPod_h

//The libraries we need
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"
#include <esp_now.h>

//Our handlers for the web server
#include "handlers/LEDRequestHandler.h"
#include "handlers/CaptiveRequestHandler.h"
#include "handlers/ServerMacAddressHandler.h"
#include "handlers/StaticHtmlHandler.h"
#include "handlers/ModeLaunchHandler.h"
#include "handlers/CSSRequestHandler.h"
#include "handlers/ScoreJSONHandler.h"

//Our control
#include "controls/ButtonControl.h"

#include "scoreStorage.h"

class ServerPod : public PhysioPod {
private :
    DNSServer* dnsServer = nullptr;
    AsyncWebServer server;
    
public :
    static ServerPod* instance;
    static const uint8_t ip_addr_broadcast[6];

    ServerPod();
    void updatePod() override;
    /* This can be called to start the specified PhysioPodMode*/
    static void startMode(PhysioPodMode* newMode);
    static void onControlPressed(); //The callback for when the control is pressed
    static void setPodLightState(uint8_t podId, bool state);
};

#endif