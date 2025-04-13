#include "PhysioPod.h"
#include "debugPrint.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

PhysioPod* PhysioPod::instance = nullptr;
CRGB PhysioPod::leds[NUM_LEDS];
CRGB PhysioPod::lightColor = CRGB::Black;
LightMode PhysioPod::lightMode = LightMode::SIMPLE;
uint16_t PhysioPod::lightModeSpecificData = 0;
bool PhysioPod::lightState = false;
bool PhysioPod::lightChanged = false;

#include <Preferences.h>

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
    DebugPrintln("Read ssid from preferences : "+ssid);
    #endif
    return ssid;
}

bool PhysioPod::searchOtherPhysioWiFi(){
    //this will try to connect to the PhysioPod network and return true if it succeeded
    //if it fails, it will return false
    #ifdef isDebug
    DebugPrintln("Trying to connect to the PhysioPod network");
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
        DebugPrintln("Connected to the PhysioPod network !");
        #endif */
        return true;
    }else{
        /* #ifdef isDebug
        DebugPrintln("Failed to connect to the PhysioPod network");
        #endif */
        return false;
    }
}

void PhysioPod::initLEDs(){
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    //create the ledTask
    #ifdef isDebug
    DebugPrintln("Creating the light manager Task");
    #endif
    xTaskCreatePinnedToCore(PhysioPod::manageOwnLight, "ledTask", 4096, NULL , 2, NULL, 0); //value is still arbitrary, but 2048 wasn't enough for Yann
}


//TODO : To avoid blocking, this uses arbitrary delays... it could be faster/slower ?
void PhysioPod::manageOwnLight(void* vParameters){
    while(true){
        if (PhysioPod::lightChanged){
            DebugPrintf("The light has changed to Mode: %d State: %d\n", PhysioPod::lightMode, PhysioPod::lightState);

            //erase Leds to start fresh
            PhysioPod::lightChanged = false;
            //DebugPrintf("Light Changed ! Clearing %d leds\n", FastLED.size());
            fill_solid(PhysioPod::leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        }
        switch (PhysioPod::lightMode){
        case LightMode::SIMPLE:
            if (PhysioPod::lightState){
                DebugPrintf("Setting the %d leds to simple color\n", FastLED.size());
                fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                FastLED.show();
            }
            while (!PhysioPod::lightChanged){
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            DebugPrintln("Exiting simple LED mode !");
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
                DebugPrintln("Exiting Blink LED mode!");
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::BLINK_SLOW:
            if (PhysioPod::lightState){
                unsigned long time = millis();
                bool microState = false;
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
                DebugPrintln("Exiting Blink LED mode!");
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
                        DebugPrintln("Exiting Cycle LED mode!");
                        break;
                    }
                    if (millis()-time>30){ //change led
                        //DebugPrintln("next led");
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
                        DebugPrintln("Exiting Cycle LED mode!");
                        break;
                    }
                    if (millis()-time>150){ //change led
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
        case LightMode::CYCLE_RAINBOW_FAST:
            if (PhysioPod::lightState){
                    uint8_t i = 0;
                    unsigned long time = millis();
                    //generate a rainbow
                    CRGB rainbow[NUM_LEDS]; 
                    fill_rainbow(rainbow, NUM_LEDS, 0, 255/NUM_LEDS);
                    while(true){
                        if (PhysioPod::lightChanged){
                            DebugPrintln("Exiting Cycle LED mode!");
                            break;
                        }
                        if (millis()-time>30){ //change led
                            //DebugPrintln("next led");
                            time = millis();
                            i = (i+1)%(NUM_LEDS);
                            PhysioPod::leds[i] = rainbow[i];
                            FastLED.show();
                            PhysioPod::leds[i] = CRGB::Black;
                        }
                        vTaskDelay(10 / portTICK_PERIOD_MS);
                    }
                }else{
                    vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
                }
                break;
        case LightMode::CYCLE_RAINBOW_SLOW:
            if (PhysioPod::lightState){
                uint8_t i = 0;
                unsigned long time = millis();
                //generate a rainbow
                CRGB rainbow[NUM_LEDS]; 
                fill_rainbow(rainbow, NUM_LEDS, 0, 255/NUM_LEDS);
                while(true){
                    if (PhysioPod::lightChanged){
                        DebugPrintln("Exiting Cycle LED mode!");
                        break;
                    }
                    if (millis()-time>150){ //change led
                        //DebugPrintln("next led");
                        time = millis();
                        i = (i+1)%(NUM_LEDS);
                        PhysioPod::leds[i] = rainbow[i];
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
                DebugPrintln("Pulse LED mode");
                FastLED.showColor(PhysioPod::lightColor);
                //fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                //FastLED.show();
                while (true){
                    if (millis()-time>20){
                        time = millis();
                        FastLED.clear(true);
                        PhysioPod::lightState = false;
                        DebugPrintln("Pulse LED mode ended, back to off");  
                        break;
                    }
                    if(PhysioPod::lightChanged){
                        DebugPrintln("Exiting Pulse LED mode!");
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
                DebugPrintln("Pulse LED mode");
                FastLED.showColor(PhysioPod::lightColor);
                //fill_solid(const_cast<CRGB*>(PhysioPod::leds), NUM_LEDS, PhysioPod::lightColor);
                //FastLED.show();
                while (true){
                    if (millis()-time>300){
                        time = millis();
                        FastLED.clear(true);
                        PhysioPod::lightState = false;
                        DebugPrintln("Pulse LED mode ended, back to off");  
                        break;
                    }
                    if(PhysioPod::lightChanged){
                        DebugPrintln("Exiting Pulse LED mode!");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::UNLOADING_BAR:
            if (PhysioPod::lightState){
                unsigned long time = millis();
                DebugPrintln("Unloading LED mode");
                FastLED.showColor(PhysioPod::lightColor);//turn on the lights
                uint8_t numUnLit = NUM_LEDS;
                while (true){
                    if ((millis()-time)/1000<lightModeSpecificData){ //lightModeSpecificData is the time (in seconds) to fill the bar
                        //progress bar, compute the number of unlit LEDs
                        uint8_t newNumUnLit = NUM_LEDS * (millis() - time) / (lightModeSpecificData * 1000);
                        if (numUnLit != newNumUnLit){
                            //DebugPrintln("Unloading LED mode, number of unlit LEDs : "+String(newNumUnLit));
                            //number has changed, update the LEDs
                            numUnLit = newNumUnLit;
                            fill_solid(PhysioPod::leds, NUM_LEDS, CRGB::Black); //turn off all the LEDs
                            fill_solid(PhysioPod::leds, NUM_LEDS-numUnLit, PhysioPod::lightColor);//turn on remaining leds
                            FastLED.show();
                        }
                        
                    }else{
                        //the bar might not be empty yet because of rounding errors
                        if (numUnLit>0){
                            //fill the rest of the bar
                            numUnLit = 0;
                            fill_solid(PhysioPod::leds, NUM_LEDS, CRGB::Black);
                            FastLED.show();
                        }
                        //DebugPrintln("Unloading LED mode, finished filling the bar");
                        //stop the loading bar
                    }
                    if(PhysioPod::lightChanged){
                        DebugPrintln("Exiting Unloading LED mode!");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        case LightMode::LOADING_BAR:
            if (PhysioPod::lightState){
                unsigned long time = millis();
                DebugPrintln("Loading LED mode");
                FastLED.showColor(CRGB::Black);//turn off the lights
                uint8_t numLit = 0;
                while (true){
                    if ((millis()-time)/1000<lightModeSpecificData){ //lightModeSpecificData is the time (in seconds) to fill the bar
                        uint8_t newNumLit = NUM_LEDS * (millis() - time) / (lightModeSpecificData * 1000);
                        if (numLit != newNumLit){
                            //number has changed, update the LEDs
                            numLit = newNumLit;
                            fill_solid(PhysioPod::leds, numLit, PhysioPod::lightColor);//turn on remaining leds
                            FastLED.show();
                        }
                    }else{
                        //the bar might not be full yet because of rounding errors
                        if (numLit<NUM_LEDS){
                            //fill the rest of the bar
                            numLit = NUM_LEDS;
                            fill_solid(PhysioPod::leds, NUM_LEDS, PhysioPod::lightColor);
                            FastLED.show();
                        }
                        //DebugPrintln("Loading LED mode, finished filling the bar");
                        //stop the loading bar
                    }
                    if(PhysioPod::lightChanged){
                        DebugPrintln("Exiting Loading LED mode!");
                        break;
                    }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }else{
                vTaskDelay(10 / portTICK_PERIOD_MS); //add a small delay, this is useful when the lights are off
            }
            break;
        
        default:
            DebugPrintln("Unknown Light mode !");
            break;
        }
/*         DebugPrint("ManageOwnLight running on core ");
        DebugPrintln(xPortGetCoreID()); */
    }
}
void PhysioPod::CreateControl(){
    #ifdef USE_CAPACITIVE_TOUCH
        DebugPrintln("Creating a Capacitive touch control");
        control = new CapacitiveTouchControl(CONTROL_PIN);
    #elif defined(USE_PROXIMITY)
        DebugPrintln("Creating a Proximity control");
        control = new ProximityControl(CONTROL_PIN);
    #elif defined(USE_PIEZO)
        DebugPrintln("Creating a Piezo control");
        control = new PiezoControl(CONTROL_PIN);
    #else
        DebugPrintln("Creating a Button control");
        control = new ButtonControl(CONTROL_PIN);
    #endif
    PhysioPodMode::setControl(control);
}

void PhysioPod::setOwnLightState(bool state, CRGB color, LightMode mode, uint16_t modeSpecific){ 
    lightState = state;
    lightColor = color;
    lightMode = mode;
    lightChanged = true;
    lightModeSpecificData = modeSpecific;
}
