#include "PhysioPodMode.h"
#include "ESPAsyncWebServer.h"
//#include <vector>

struct ChaseParameters{
    int cycles;
    uint8_t cycle[512];
    int cycleLength;
}; 

class ChaseMode : public PhysioPodMode {
protected :
    long startTimer;
    int currentStep;
    int8_t currentCycle;
    int8_t cycles;
    int cycleLength;
    uint8_t cycle[512];
    static ChaseParameters parameters;   
    
public:
    void initialize(int cycles, uint8_t* cycle, int cycleLength);

    static bool testRequestParameters(AsyncWebServerRequest *request);
    static PhysioPodMode* generateMode();
    
    const char* getName() override { return "ChaseMode"; }

    void start() override;
    void stop() override;
    void update() override;
    void reset() override;
    String* returnScore() override;
    void onPodPressed(uint8_t id) override;

    ChaseMode();
};