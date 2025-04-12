#include <Arduino.h>
#include "PhysioPodControl.h"
#include "PiezoControl.h"
#include "debugPrint.h"

#ifndef PIEZO_THRESHOLD
#define PIEZO_THRESHOLD 100 //default value, should be adjusted in platformio.ini
#endif

#define debounceDelay 50

PiezoControl::PiezoControl(byte pin){
    this->pin = pin;
    //this->checking = false;
    this->state = false;
    this->lastDebounceTime = 0;
    this->onPressedCallback = nullptr;
    DebugPrintln("PiezoControl created");
}

void PiezoControl::initialize(void (*callback)()){
    this->state = false;
    this->lastDebounceTime = millis();
    this->onPressedCallback = callback;
}


bool PiezoControl::checkControl(){
    bool newState = analogRead(pin)> PIEZO_THRESHOLD;
    /* DebugPrint(">piezzo:");
    DebugPrintln(analogRead(pin)); */
    if (newState!=state){
        DebugPrintf(">piezzo:%d\n", analogRead(pin));    
        //check if enough time has passed since the last change
        if (millis() - lastDebounceTime > debounceDelay){
            lastDebounceTime = millis();
            state = !state;
            #ifdef isDebug
            DebugPrint("Piezo state changed : ");
            DebugPrintln(state ? "HIGH" : "LOW");
            #endif
            //notify the pod that the button is pressed
            if (!state){
                onPressedCallback();
            }
        }
        #ifdef isDebug
        DebugPrintln("Ignored a bounce !");
        #endif
    }
    return state;
}