#include <Arduino.h>
#include "PhysioPodControl.h"
#include "ButtonControl.h"
#include "isDebug.h"

#define debounceDelay 50

ButtonControl::ButtonControl(byte pin){
    this->pin = pin;
    this->checking = false;
    this->state = false;
}

void ButtonControl::initialize(void (*callback)()){
    this->checking = true;
    pinMode(pin, INPUT_PULLUP);
    this->state = digitalRead(pin);
    this->lastDebounceTime = millis();
    this->onPressedCallback = callback;
}

void ButtonControl::stop(){
    this->checking = false;
}

bool ButtonControl::checkControl(){
    //Serial.println("Checking the button control");
    if (this->checking){
        if (digitalRead(pin)!=state){
            //check if enough time has passed since the last change
            if (millis() - lastDebounceTime > debounceDelay){
                lastDebounceTime = millis();
                state = !state;
                #ifdef isDebug
                Serial.print("Button state changed : ");
                Serial.println(state ? "HIGH" : "LOW");
                //notify the pod that the button is pressed
                if (state){
                    onPressedCallback();
                }
                #endif
                return !state; //return true if the button is pressed
            }
            #ifdef isDebug
            Serial.println("Ignored a bounce !");
            #endif
        }
        return !state;
    }
    //what is the point of calling checkControl if the control is not checking ? 
    //TODO : remove checking state ?
    return false;
}