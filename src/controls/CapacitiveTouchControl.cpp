#include <Arduino.h>
#include "PhysioPodControl.h"
#include "CapacitiveTouchControl.h"
#include "isDebug.h"

#define CAPACITIVE_TOUCH_THRESHOLD 40000
#define debounceDelay 50

CapacitiveTouchControl::CapacitiveTouchControl(byte pin){
    this->pin = pin;
    //this->checking = false;
    this->state = false;
    this->onPressedCallback = nullptr;
}

void CapacitiveTouchControl::initialize(void (*callback)()){
    this->onPressedCallback = callback;
    this->lastDebounceTime = millis();
    #ifndef USE_CAPACITIVE_TOUCH
    Serial.println("You are initializing a Capacitive touch, please enable USE_CAPACITIVE_TOUCH build flag in platformio.ini");
    #endif
}

bool CapacitiveTouchControl::checkControl(){
    #ifdef USE_CAPACITIVE_TOUCH
    //Serial.println(touchRead(pin));
    #ifdef USE_INVERTED_TOUCH
        bool newState=(touchRead(pin) > CAPACITIVE_TOUCH_THRESHOLD);
    #else
        bool newState=(touchRead(pin) < CAPACITIVE_TOUCH_THRESHOLD);
    #endif
    
    if (state != newState){
        if (millis() - lastDebounceTime > debounceDelay){
            lastDebounceTime = millis();
            state = !state;
            #ifdef isDebug
            Serial.print("Capacitive touch state changed : ");
            Serial.println(state ? "HIGH" : "LOW");
            #endif
            if (state){
                //notify the pod that the button is pressed
                onPressedCallback();
            }
        }
        #ifdef isDebug
        else{
            Serial.println("Ignored a touch event due to debounce delay");
        }
        #endif
    }
    #endif
    return state; //return true if the button is pressed
    
}