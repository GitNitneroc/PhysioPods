#include "PhysioPodMode.h"
#include "ESPAsyncWebServer.h"
//#include <vector>

struct ChaseParameters{
    int cycles;
    uint8_t cycle[512];
    uint8_t colors[512]; //those colors are in range 0-7 and should be converted on the HSV scale
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
    uint8_t colors[512];
    static ChaseParameters parameters;

    void lightCurrentPod();
    
public:
    void initialize(int cycles, uint8_t* cycle, uint8_t* colors, int cycleLength);

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