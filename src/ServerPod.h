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
    static void DNSLoop(void * vpParameters);
    
    bool staticFilesEnabled = false; //if false, the static files are not served
    void attachHandlers(AsyncWebServer* server);
    void prepareCaptivePortal(AsyncWebServer *server, bool sendToOTA = false);
    bool initializeSPIFFS(); //tries to initialize SPIFFS, and returns true if index.html is found

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
    static void setPodLightState(uint8_t podId, bool state, CRGB color = CRGB::White, LightMode mode = LightMode::SIMPLE, uint16_t modeSpecific = 0);
    static void setPodBuzzerState(uint8_t podId, bool state, uint16_t frequency = 0, uint16_t duration = 0);
    /* static void broadcastMessage(const void* message); */

    //to please cppcheck :
    ServerPod(const ServerPod&) = delete;
    ServerPod& operator=(const ServerPod&) = delete;
};

#endif