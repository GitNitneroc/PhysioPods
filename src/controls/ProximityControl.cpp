#include <Arduino.h>
#include "PhysioPodControl.h"
#include "ProximityControl.h"

#define debounceDelay 50

ProximityControl::ProximityControl(byte pin){
    this->pin = pin;
    this->state = false;
    this->lastDebounceTime = 0;
    this->onPressedCallback = nullptr;
}

void ProximityControl::initialize(void (*callback)()){
    //this->checking = true;
    pinMode(pin, INPUT);
    this->state = digitalRead(pin);
    this->lastDebounceTime = millis();
    this->onPressedCallback = callback;
}

bool ProximityControl::checkControl(){

    if (digitalRead(pin)!=state){
        //check if enough time has passed since the last change
        if (millis() - lastDebounceTime > debounceDelay){
            lastDebounceTime = millis();
            state = !state;
            #ifdef isDebug
            Serial.print("Proximity state changed : ");
            Serial.println(state ? "HIGH" : "LOW");
            #endif
            //notify the pod that the button is pressed
            if (!state){
                onPressedCallback();
            }
            return !state; //return true if the current is low (INPUT_PULLUP)
        }
        #ifdef isDebug
        Serial.println("Ignored a bounce !");
        #endif
    }
    return !state;
}