#ifndef ProximityControl_h
#define ProximityControl_h
#include <Arduino.h>
#include "PhysioPodControl.h"

/*
    An IR proximity sensor control class, to use with a KY-032 / FC-51 sensor
*/
class ProximityControl: public PhysioPodControl {
    public:
        explicit ProximityControl(byte pin);
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