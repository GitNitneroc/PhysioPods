#ifndef PhysioPodMode_h
#define PhysioPodMode_h
#include <Arduino.h>

class PhysioPodMode {
public:
    //virtual void initialize() = 0;
    virtual void stop() = 0;

    virtual ~PhysioPodMode() {}

    virtual void update() = 0;
    virtual void reset() = 0;
    virtual String returnScore() = 0;
};

#endif