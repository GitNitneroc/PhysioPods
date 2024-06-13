#include <Arduino.h>
#include "PhysioPodControl.h"
#include "PiezoControl.h"

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
}

void PiezoControl::initialize(void (*callback)()){
    this->state = false;
    this->lastDebounceTime = millis();
    this->onPressedCallback = callback;
}


bool PiezoControl::checkControl(){
    bool newState = analogRead(pin)> PIEZO_THRESHOLD;
    //TODO : remove this debug
    Serial.print(">piezzo:");
    Serial.println(analogRead(pin));
    if (newState!=state){
        //check if enough time has passed since the last change
        if (millis() - lastDebounceTime > debounceDelay){
            lastDebounceTime = millis();
            state = !state;
            #ifdef isDebug
            Serial.print("Piezo state changed : ");
            Serial.println(state ? "HIGH" : "LOW");
            #endif
            //notify the pod that the button is pressed
            if (state){
                onPressedCallback();
            }
        }
        #ifdef isDebug
        Serial.println("Ignored a bounce !");
        #endif
    }
    return state;
}