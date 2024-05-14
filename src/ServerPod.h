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

#include "modes/PhysioPodMode.h"

#include "scoreStorage.h"

class ServerPod : public PhysioPod {
private :
    DNSServer* dnsServer = nullptr;
    static void OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len);
    static void SendPong(uint8_t podId);
    static void OnAPStart(WiFiEvent_t event, WiFiEventInfo_t info);
    static void CheckClientTimeouts(void * vpParameters);

public :
    uint8_t clientsTimers[254]; //no need for 255 since id 0 is self 
    static const uint8_t ip_addr_broadcast[6];
    static uint8_t peersNum; //number of peers connected to the server
    AsyncWebServer server;
    bool APStarted = false;

    static ServerPod* getInstance(){
        return (ServerPod*)instance;
    }

    ServerPod();
    void updatePod() override;
    /* This can be called to start the specified PhysioPodMode*/
    static void startMode(PhysioPodMode* newMode);
    static void onControlPressed(); //The callback for when the control is pressed
    static void setPodLightState(uint8_t podId, bool state, CRGB color = CRGB::White, LightMode mode = LightMode::SIMPLE);

    //to please cppcheck :
    ServerPod(const ServerPod&) = delete;
    ServerPod& operator=(const ServerPod&) = delete;
};

#endif