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

CRGB PhysioPod::lightColor = CRGB::Black;
LightMode PhysioPod::lightMode = LightMode::SIMPLE;
bool PhysioPod::lightState = false;
bool PhysioPod::lightChanged = false;


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

    //create the ledTask
    #ifdef isDebug
    Serial.println("Creating the light manager Task");
    #endif
    xTaskCreate(PhysioPod::manageOwnLight, "ledTask", 2048, NULL , 2, &ledTask);
}


//TODO : To avoid blocking, this uses arbitrary delays... it could be faster/slower ?
void PhysioPod::manageOwnLight(void* vParameters){
    while(true){
        if (PhysioPod::lightChanged){
            printf("The light has changed to Mode: %d State: %d\n", PhysioPod::lightMode, PhysioPod::lightState);

            //erase Leds to start fresh
            PhysioPod::lightChanged = false;
            //Serial.printf("Light Changed ! Clearing %d leds\n", FastLED.size());
            fill_solid(PhysioPod::leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        }
        switch (PhysioPod::lightMode){
        case LightMode::SIMPLE:
            if (PhysioPod::lightState){
                Serial.printf("Setting the %d leds to simple color\n", FastLED.size());
                fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                FastLED.show();
            }
            while (!PhysioPod::lightChanged){
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            Serial.println("Exiting simple LED mode !");
            break;
        case LightMode::BLINK_FAST:
            if (PhysioPod::lightState){
                unsigned long time = millis();
                bool microState = true;
                while(!PhysioPod::lightChanged){
                    if (millis()-time>150){
                        //blink
                        time = millis();
                        microState = !microState;
                        if (microState){
                            fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                            FastLED.show();
                        }else{
                            fill_solid(PhysioPod::leds, NUM_LEDS, CRGB::Black);
                            FastLED.show();
                        }
                    }else{
                        vTaskDelay(10 / portTICK_PERIOD_MS); //just wait till next blink
                    }
                }
                Serial.println("Exiting Blink LED mode!");
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::BLINK_SLOW:
            if (PhysioPod::lightState){
                unsigned long time = millis();
                bool microState = true;
                while(!PhysioPod::lightChanged){
                    if (millis()-time>500){
                        //blink
                        time = millis();
                        microState = !microState;
                        if (microState){
                            fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                            FastLED.show();
                        }else{
                            fill_solid(PhysioPod::leds, NUM_LEDS, CRGB::Black);
                            FastLED.show();
                        }
                    }else{
                        vTaskDelay(10 / portTICK_PERIOD_MS); //just wait till next blink
                    }
                }
                Serial.println("Exiting Blink LED mode!");
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::CYCLE_FAST:
            if (PhysioPod::lightState){
                uint8_t i = 0;
                unsigned long time = millis();
                while(true){
                    if (PhysioPod::lightChanged){
                        Serial.println("Exiting Cycle LED mode!");
                        break;
                    }
                    if (millis()-time>30){ //change led
                        //Serial.println("next led");
                        time = millis();
                        i = (i+1)%(NUM_LEDS);
                        PhysioPod::leds[i] = PhysioPod::lightColor;
                        FastLED.show();
                        PhysioPod::leds[i] = CRGB::Black;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::CYCLE_SLOW:
            if (PhysioPod::lightState){
                uint8_t i = 0;
                unsigned long time = millis();
                while(true){
                    if (PhysioPod::lightChanged){
                        Serial.println("Exiting Cycle LED mode!");
                        break;
                    }
                    if (millis()-time>200){ //change led
                        time = millis();
                        i = (i+1)%(NUM_LEDS);
                        PhysioPod::leds[i] = PhysioPod::lightColor;
                        FastLED.show();
                        PhysioPod::leds[i] = CRGB::Black;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::PULSE_ON_OFF_SHORT :     
            if (PhysioPod::lightState){
                unsigned long time = millis();
                Serial.println("Pulse LED mode");
                fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                FastLED.show();
                while (true){
                    if (millis()-time>20){
                        time = millis();
                        FastLED.clear(true);
                        PhysioPod::lightState = false;
                        Serial.println("Pulse LED mode ended, back to off");  
                        break;
                    }
                    if(PhysioPod::lightChanged){
                        Serial.println("Exiting Pulse LED mode!");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::PULSE_ON_OFF_LONG :
            if (PhysioPod::lightState){
                unsigned long time = millis();
                Serial.println("Pulse LED mode");
                fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                FastLED.show();
                while (true){
                    if (millis()-time>300){
                        time = millis();
                        FastLED.clear(true);
                        PhysioPod::lightState = false;
                        Serial.println("Pulse LED mode ended, back to off");  
                        break;
                    }
                    if(PhysioPod::lightChanged){
                        Serial.println("Exiting Pulse LED mode!");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        
        default:
            Serial.println("Unknown Light mode !");
            break;
        }
/*         Serial.print("ManageOwnLight running on core ");
        Serial.println(xPortGetCoreID()); */
    }
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
    #ifdef isDebug
    Serial.print(">Available memory:");
    Serial.println(ESP.getFreeHeap());
    Serial.printf("-Setting the rgb led to state %d, with color : %d,%d,%d. Mode = %d\n", state, color.r, color.g, color.b, mode);
    #endif

    PhysioPod::lightState = state;
    PhysioPod::lightColor = color;
    PhysioPod::lightMode = mode;
    PhysioPod::lightChanged = true;
}
