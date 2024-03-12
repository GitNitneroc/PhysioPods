#include <Arduino.h>
#include "PhysioPodControl.h"
#include "CapacitiveTouchControl.h"
#include "isDebug.h"

#define CAPACITIVE_TOUCH_THRESHOLD 100000

CapacitiveTouchControl::CapacitiveTouchControl(byte pin){
    this->pin = pin;
    //this->checking = false;
    this->state = false;
    this->onPressedCallback = nullptr;
}

void CapacitiveTouchControl::initialize(void (*callback)()){
    this->onPressedCallback = callback;
}

bool CapacitiveTouchControl::checkControl(){
    #ifdef USE_INVERTED_TOUCH
        bool newState=(touchRead(pin) > CAPACITIVE_TOUCH_THRESHOLD);
    #else
        bool newState=(touchRead(pin) < CAPACITIVE_TOUCH_THRESHOLD);
    #endif
    if (state != newState){
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
    return state; //return true if the button is pressed
    
}