#include <Arduino.h>
#include "PhysioPodControl.h"
#include "ButtonControl.h"

ButtonControl::ButtonControl(byte pin){
    this->pin = pin;
    this->checking = false;
}

void ButtonControl::initialize(){
    this->checking = true;
    pinMode(pin, INPUT_PULLUP);
}

void ButtonControl::stop(){
    this->checking = false;
}

bool ButtonControl::checkControl(){
    Serial.println("Checking the button control");
    if (this->checking){
        return digitalRead(pin) == LOW;
    }
    return false;
}