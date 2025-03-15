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

class VisualTimerMode : public PhysioPodMode {
    private:
        long timer; // Utile ?
        long interval; // Utile ?
        long restTime; 
        long workTime;
        long pressDelay; // Utile ?
        uint numberOfCycles;
        uint currentCircle;
        int score; // Utile ?
        uint errors; // Utile ?
        uint8_t podToPress; // Utile ?
    
        //void updatePodToPress(); // Utile ?
        //void onError(uint8_t pod); // Utile ?
        //void onSuccess(uint8_t pod); // Utile ?
        void onPodPressed(uint8_t id) override; // Utile ?
    
    public:
        static VisualTimerModeParameters parameters;
    
        static bool testRequestParameters(AsyncWebServerRequest *request);
        static PhysioPodMode* generateMode();
        void initialize(long workTime, long restTime, uint8_t numberOfCycles); 
        void start();
        void stop();
        void update();
        void reset();
        String* returnScore(); // Utile ?
        const char* getName() override { return "VisualTimer"; }
    
        VisualTimerMode();
        virtual ~VisualTimerMode() {}
    };

#endif // VISUALTIMERMODE_H