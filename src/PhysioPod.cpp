#include "PhysioPod.h"
#include "isDebug.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

bool PhysioPod::searchOtherPhysioWiFi(){
    #ifdef isDebug
    Serial.println("Scanning for other PhysioPods...");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    bool found = false;
    Serial.println("Found "+String(n)+" networks");
    //TODO : this could help to find the best channel
    for (int i = 0; i < n; i++){
        #ifdef isDebug
        Serial.print(" - "+WiFi.SSID(i));
        Serial.println(" (channel "+String(WiFi.channel(i))+")");
        #endif
        if (WiFi.SSID(i) == ssid){
            #ifdef isDebug
            Serial.println(" ! Found a PhysioPod network !");
            #endif
            found = true;
            //break; We could break here, but displaying the channel could be interesting
        }
    }
    WiFi.scanDelete();
    WiFi.disconnect();
    delay(100);
    return found;
}

void PhysioPod::setLightState(bool state) {
    #ifdef isDebug
        Serial.println("Setting the light to "+String(state));
    #endif
    #ifdef USE_NEOPIXEL
        if (state){
            neopixelWrite(LED_PIN,100,20,20); // on
        }
        else{
            neopixelWrite(LED_PIN,0,0,0); // off
        }
    #else
        #ifdef INVERTED_LED
        state = !state;
        #endif
        digitalWrite(LED_PIN, state);
    #endif
}