#ifndef VISUALTIMERMODE_H
#define VISUALTIMERMODE_H

#include "PhysioPodMode.h"
#include "ESPAsyncWebServer.h"
#include <FastLED.h>

struct VisualTimerModeParameters {
    long workTime;
    long restTime;
    uint8_t numberOfCycles;
};

enum VisualTimerState {
    WORKING,
    RESTING,
    IDLE, //don't know if this is useful
};

class VisualTimerMode : public PhysioPodMode {
    private:
        long timer; // Utile ?

        long restTime; 
        long workTime;

        VisualTimerState state;

        uint numberOfCycles;
        uint currentCircle;
    
    public:
        static VisualTimerModeParameters parameters;
    
        static bool testRequestParameters(AsyncWebServerRequest *request);
        static PhysioPodMode* generateMode();
        void initialize(long workTime, long restTime, uint8_t numberOfCycles); 
        void start();
        void stop();
        void update();
        void reset();
        String* returnScore();
        const char* getName() override { return "VisualTimer"; }
    
        VisualTimerMode();
        virtual ~VisualTimerMode() {}
    };

#endif // VISUALTIMERMODE_H