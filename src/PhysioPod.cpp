#include "PhysioPod.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

PhysioPod* PhysioPod::instance = nullptr;

#include <Preferences.h>

#ifdef USE_NEOPIXEL
//initialize the static members for the leds
CRGB PhysioPod::leds[NUM_LEDS];
TaskHandle_t PhysioPod::ledTask = NULL;
#endif

uint16_t PhysioPod::getSessionId(){
    return sessionId;
}

String PhysioPod::getSSIDFromPreferences(){
    Preferences preferences;    
    preferences.begin("PhysioPod", true);
    uint ssidNum = preferences.getUInt("ssid", 1);
    preferences.end();
    String ssid = "PhysioPods";
    ssid.concat(ssidNum);
    #ifdef isDebug
    Serial.println("Read ssid from preferences : "+ssid);
    #endif
    return ssid;
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

    //get the ssid from the preferences
    char ssid[32];
    strcpy(ssid, getSSIDFromPreferences().c_str());
    WiFi.begin(ssid, password);
    bool LEDState = true;

    //limit connection time
    unsigned long start = millis();
    setOwnLightState(true, CRGB::Green, LightMode::CYCLE_FAST);
    while (WiFi.status() != WL_CONNECTED && millis()-start<CONNECTION_TIMEOUT){
        vTaskDelay(100 / portTICK_PERIOD_MS);
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

/*
    * This creates the led or leds for the pod
*/
void PhysioPod::initLEDs(){
    #ifdef USE_NEOPIXEL
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    #else
    pinMode(LED_PIN, OUTPUT);
    #endif
}

void PhysioPod::CreateControl(){
    #ifdef USE_CAPACITIVE_TOUCH
        control = new CapacitiveTouchControl(BUTTON_PIN);
    #elif defined(USE_PROXIMITY)
        control = new ProximityControl(BUTTON_PIN);
    #elif defined(USE_PIEZO)
        control = new PiezoControl(BUTTON_PIN);
    #else
        control = new ButtonControl(BUTTON_PIN);
    #endif
    PhysioPodMode::setControl(control);
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

    //remark : this could help to find the best channel
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

void PhysioPod::setOwnLightState(bool state, CRGB color, LightMode mode) {
    #ifndef USE_NEOPIXEL
        #ifdef isDebug
        Serial.println("The neopixel is not enabled, the color will not be set");
        #endif
        #ifdef INVERTED_LED
            state = !state;
        #endif
        return digitalWrite(LED_PIN, state);
    #endif

    #ifdef isDebug
        Serial.printf("Setting the rgb led to state %d, with color : %d,%d,%d. Mode = %d\n", state, color.r, color.g, color.b, mode);
    #endif

    //we are using neopixels
    //kill the ledTask if it was running
    if (ledTask != NULL){
        vTaskDelete(ledTask);
        ledTask = NULL;
        #ifdef isDebug
        Serial.println("Killed the ledTask");
        #endif
    }

    //clear the leds
    FastLED.clear(true);
    
    if (state){
        switch (mode){
        case LightMode::SIMPLE:
            FastLED.showColor(color);
            //FastLED.show();
            break;
        case LightMode::BLINK_FAST:
            //create the blink task
            xTaskCreate(PhysioPod::FastBlinkLeds, "ledTask", 2048, (void *) &color, 2, &ledTask);
            break;
        case LightMode::BLINK_SLOW:
            //create the blink task
            xTaskCreate(PhysioPod::SlowBlinkLeds, "ledTask", 2048, (void *) &color, 2, &ledTask);
            break;
        case LightMode::CYCLE_FAST:
            //create the cycle task
            xTaskCreate(PhysioPod::FastCycleLeds, "ledTask", 2048, (void *) &color, 2, &ledTask);
            break;
        case LightMode::CYCLE_SLOW:
            //create the cycle task
            xTaskCreate(PhysioPod::SlowCycleLeds, "ledTask", 2048, (void *) &color, 2, &ledTask);
            break;
        case LightMode::PULSE_ON_OFF_SHORT:
            //create the pulse task
            xTaskCreate(PhysioPod::ShortPulseLeds, "ledTask", 2048, (void *) &color, 2, &ledTask);
            break;
        case LightMode::PULSE_ON_OFF_LONG:
            //create the pulse task
            xTaskCreate(PhysioPod::LongPulseLeds, "ledTask", 2048, (void *) &color, 2, &ledTask);
            break;
        default:
            break;
        }
    }
    else{
        //the leds have been cleared
        //the ledTask is not running
        FastLED.show();
    }
}

void PhysioPod::ShortPulseLeds(void* param){
    CRGB color = *(static_cast<CRGB*>(param));
    PhysioPod::PulseLeds(color, 20);
}

void PhysioPod::LongPulseLeds(void* param){
    CRGB color = *(static_cast<CRGB*>(param));
    PhysioPod::PulseLeds(color, 200);
}

void PhysioPod::PulseLeds(CRGB color, long delayTime){
    uint8_t i = 0;
    FastLED.showColor(color);
    vTaskDelay(delayTime / portTICK_PERIOD_MS);
    FastLED.clear(true);
    FastLED.show();
    #ifdef isDebug
    Serial.println("Pulse has ended, leds off now");
    #endif
    //kill the ledTask
    PhysioPod::ledTask = NULL;
    vTaskDelete(nullptr);
}

void PhysioPod::FastCycleLeds(void* param){
    CRGB color = *(static_cast<CRGB*>(param));
    PhysioPod::CycleLeds(color, 20);
}

void PhysioPod::SlowCycleLeds(void* param){
    CRGB color = *(static_cast<CRGB*>(param));
    PhysioPod::CycleLeds(color, 100);
}

void PhysioPod::CycleLeds(CRGB color, long delayTime){
    uint8_t i = 0;
    //TODO : this only supports USE_NEOPIXEL
    while(true){
        i = (i+1)%(NUM_LEDS);
        PhysioPod::leds[i] = color;
        FastLED.show();
        //Serial.println("Cycle is showing the leds");
        vTaskDelay(delayTime / portTICK_PERIOD_MS);
        PhysioPod::leds[i] = CRGB::Black;
    }
}

void PhysioPod::FastBlinkLeds(void* param){
    CRGB color = *(static_cast<CRGB*>(param));
    PhysioPod::BlinkLeds(color, 200);
}

void PhysioPod::SlowBlinkLeds(void* param){
    CRGB color = *(static_cast<CRGB*>(param));
    PhysioPod::BlinkLeds(color, 500);
}

void PhysioPod::BlinkLeds(CRGB color, long delayTime){
    uint8_t i = 0;
    while(true){
        FastLED.showColor(color);
        //Serial.println("Cycle is showing the leds");
        vTaskDelay(delayTime / portTICK_PERIOD_MS);
        FastLED.clear(true);
        FastLED.show();
        vTaskDelay(delayTime / portTICK_PERIOD_MS);
    }
}
