#include <Arduino.h>
#include "PhysioPodControl.h"

/*
    A simple button control.
    This control is used to check if a button is pressed.
    The button should be connected to the ground and the pin. A PULLUP resistor is used
*/
class ButtonControl: public PhysioPodControl {
    public:
        ButtonControl(byte pin);
        void initialize();
        void stop();
        bool checkControl();
    private:
        byte pin;
        bool checking;
        bool state;
        ulong lastDebounceTime;
};