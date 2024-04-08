#ifndef CAPACITIVETOUCHCONTROL_H
#define CAPACITIVETOUCHCONTROL_H
#include <Arduino.h>
#include "PhysioPodControl.h"

class CapacitiveTouchControl : public PhysioPodControl {
    public:
        CapacitiveTouchControl(byte pin);
        void initialize(void (*callback)());
        //void stop();
        bool checkControl();

    private:
        byte pin;
        //bool checking;
        bool state;
        long lastDebounceTime = 0;
};

#endif // CAPACITIVETOUCHCONTROL_H