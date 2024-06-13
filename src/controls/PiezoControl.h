#ifndef PiezoControl_h
#define PiezoControl_h
#include <Arduino.h>
#include "PhysioPodControl.h"

/*
    A simple button control.
    This control is used to check if a button is pressed.
    The button should be connected to the ground and the pin. A PULLUP resistor is used
*/
class PiezoControl: public PhysioPodControl {
    public:
        explicit PiezoControl(byte pin);
        void initialize(void (*callback)()) override;
        //void stop();
        bool checkControl() override;
    private:
        byte pin;
        //bool checking;
        bool state;
        ulong lastDebounceTime;
};
#endif