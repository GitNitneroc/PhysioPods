#include <Arduino.h>
#include "PhysioPodControl.h"
#include "ProximityControl.h"
#include "debugPrint.h"

#define debounceDelay 50

ProximityControl::ProximityControl(byte pin){
    this->pin = pin;
    this->state = false;
    this->lastDebounceTime = 0;
    this->onPressedCallback = nullptr;
    DebugPrintln("ProximityControl created");
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
            DebugPrint("Proximity state changed : ");
            DebugPrintln(state ? "HIGH" : "LOW");
            #endif
            //notify the pod that the button is pressed
            if (!state){
                onPressedCallback();
            }
            return !state; //return true if the current is low (INPUT_PULLUP)
        }
        #ifdef isDebug
        DebugPrintln("Ignored a bounce !");
        #endif
    }
    return !state;
}