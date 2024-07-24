#include <Arduino.h>
#include "PhysioPodControl.h"
#include "ButtonControl.h"

#define debounceDelay 50

ButtonControl::ButtonControl(byte pin){
    this->pin = pin;
    //this->checking = false;
    this->state = false;
    this->lastDebounceTime = 0;
    this->onPressedCallback = nullptr;
    Serial.println("ButtonControl created");
}

void ButtonControl::initialize(void (*callback)()){
    //this->checking = true;
    pinMode(pin, INPUT_PULLUP);
    this->state = digitalRead(pin);
    this->lastDebounceTime = millis();
    this->onPressedCallback = callback;
}

/* void ButtonControl::stop(){
    this->checking = false;
} */

bool ButtonControl::checkControl(){
    //Serial.println("Checking the button control");
    //if (this->checking){
        if (digitalRead(pin)!=state){
            //check if enough time has passed since the last change
            if (millis() - lastDebounceTime > debounceDelay){
                lastDebounceTime = millis();
                state = !state;
                #ifdef isDebug
                Serial.print("Button state changed : ");
                Serial.println(state ? "HIGH" : "LOW");
                #endif
                //notify the pod that the button is pressed
                if (state){
                    onPressedCallback();
                }
                return !state; //return true if the current is low (INPUT_PULLUP)
            }
            #ifdef isDebug
            Serial.println("Ignored a bounce !");
            #endif
        }
        return !state;
    //}
}