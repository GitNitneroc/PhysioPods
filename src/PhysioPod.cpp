#include "PhysioPod.h"
#include "isDebug.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

//TODO : Introduce a keep-alive espnow message, sent by the server... and make the client respond to it, or just restart if no keep-alive is received for a while

PhysioPod* PhysioPod::instance = nullptr;

uint16_t PhysioPod::getSessionId(){
    return sessionId;
}

bool PhysioPod::searchOtherPhysioWiFi(){
    //this will try to connect to the PhysioPod network and return true if it succeeded
    //if it fails, it will return false
    #ifdef isDebug
    Serial.println("Trying to connect to the PhysioPod network");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    bool LEDState = true;
    //limit connection time
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis()-start<CONNECTION_TIMEOUT){
        #ifdef USE_NEOPIXEL
        setOwnLightState(LEDState, 4,75,13);
        #else
        setOwnLightState(LEDState);
        #endif
        LEDState = !LEDState;
        delay(200);
    }
    setOwnLightState(false);
    if (WiFi.status() == WL_CONNECTED){
        /* #ifdef isDebug
        Serial.println("Connected to the PhysioPod network !");
        #endif */
        return true;
    }else{
        /* #ifdef isDebug
        Serial.println("Failed to connect to the PhysioPod network");
        #endif */
        return false;
    }
}

/* older version of the searchOtherPhysioWiFi function, that used a wifi scan... it was too slow, but could be interesting to select the best channel
bool PhysioPod::SearchOtherPhysioWiFi(){
    #ifdef isDebug
    Serial.println("Scanning for other PhysioPods...");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.scanNetworks(true);
    bool LEDState = true;
    int n=-2;
    while (true){ 
        n=WiFi.scanComplete();//-2 means it was not asked to scan, -1 means it is scanning, 0+ are the number of networks found
        if (n>=0){
            break;
        }
        #ifdef USE_NEOPIXEL
        setOwnLightState(LEDState, 4,75,13);
        #else
        setOwnLightState(LEDState);
        #endif
        LEDState = !LEDState;
        delay(200);
    }
    setOwnLightState(false);
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
*/

void PhysioPod::setOwnLightState(bool state, uint8_t r, uint8_t g, uint8_t b) {
    #ifndef USE_NEOPIXEL
        Serial.println("The neopixel is not enabled, the color will not be set");
        #ifdef INVERTED_LED
            state = !state;
        #endif
        return digitalWrite(LED_PIN, state);
    #endif

    #ifdef isDebug
        Serial.println("Setting the rgb led to "+String(state));
    #endif
    
    if (state){
        neopixelWrite(LED_PIN,r,g,b); // on
    }
    else{
        neopixelWrite(LED_PIN,0,0,0); // off
    }
}
