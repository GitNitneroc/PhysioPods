#include "PhysioPodMode.h"
//#include <vector>


class FairyLightsMode : public PhysioPodMode {
protected :
    int timeByPod;
    long timer;
    uint8_t litPod;
    
public:
    void initialize(int timeByPod);
    
    const char* getName() override { return "FairyLightsMode"; }

    void start() override;
    void stop() override;
    void update() override;
    void reset() override;
    String* returnScore() override;
    void onPodPressed(uint8_t id) override;

    FairyLightsMode();
};