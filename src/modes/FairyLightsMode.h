#include "PhysioPodMode.h"
#include "ESPAsyncWebServer.h"
//#include <vector>

struct FairyLightsParameters{
    int timeByPod;
}; 

class FairyLightsMode : public PhysioPodMode {
protected :
    int timeByPod;
    long timer;
    uint8_t litPod;
    static FairyLightsParameters parameters;   
    
public:
    void initialize(int timeByPod);

    static bool testRequestParameters(AsyncWebServerRequest *request);
    static PhysioPodMode* generateMode();
    
    const char* getName() override { return "FairyLightsMode"; }

    void start() override;
    void stop() override;
    void update() override;
    void reset() override;
    String* returnScore() override;
    void onPodPressed(uint8_t id) override;

    FairyLightsMode();
};