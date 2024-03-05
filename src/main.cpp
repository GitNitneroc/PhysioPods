#include "isDebug.h" //this file is used to define the isDebug variable
#include "ServerPod.h"
#include "ClientPod.h"

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

#include "scoreStorage.h"


PhysioPod* pod = nullptr;


void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    //delay(7000); //My esp module Serial is messed up after upload, so I need to wait for it to boot up
    Serial.println("Booting");
    #endif

    //initialize the LED
    pinMode(LED_PIN, OUTPUT);

    /* // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100); */
   

    if (PhysioPod::searchOtherPhysioWiFi()){
        pod = new ClientPod();
    }else{
        ScoreStorage::init();
        pod = new ServerPod();
    }

    //make sure the light is off
    pod->setLightState(false);
}

void loop(){
    pod->updatePod();

    //TODO : consider a timer to go easy on the battery ?
}