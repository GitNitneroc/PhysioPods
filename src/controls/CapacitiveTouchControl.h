#ifndef CAPACITIVETOUCHCONTROL_H
#define CAPACITIVETOUCHCONTROL_H
#include <Arduino.h>
#include "PhysioPodControl.h"

class CapacitiveTouchControl : public PhysioPodControl {
    public:
        explicit CapacitiveTouchControl(byte pin);
        void initialize(void (*callback)()) override;
        //void stop();
        bool checkControl() override;

    private:
        byte pin;
        //bool checking;
        bool state;
        long lastDebounceTime = 0;
};

#endif // CAPACITIVETOUCHCONTROL_H