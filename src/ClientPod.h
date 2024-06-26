#include "PhysioPod.h"

//The libraries we need

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <esp_now.h>

//Our control
#include "controls/ButtonControl.h"

class ClientPod : public PhysioPod {
private :
    uint8_t serverMac[6];
    static void OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len);
    static uint8_t serverTimer; //this counts the number of times the server didn't send a pong

public :
    ClientPod();
    void updatePod() override;
    static void onControlPressed(); //The callback for when the control is pressed
    void displayError();
    static ClientPod* getInstance(){
        return (ClientPod*)instance;
    }
    static void PingServer(void * pvParameters);
};
