#include "PhysioPodMode.h"
#include <Arduino.h>
#include "controls/PhysioPodControl.h"
#include "scoreStorage.h"

enum State{
    STOPPED,
    DURING_INTERVAL,
    WAIT_FOR_PRESS,
    WAIT_FOR_RELEASE
};

/*
    This mode is a simple game where the user has to press the button as soon as possible after the LED turns on.
*/
class FastPressMode : public PhysioPodMode {
private:
    long timer;
    long interval;
    long minInterval;
    long maxInterval;
    long pressDelay;
    uint numberOfTries;
    uint currentTry;
    int score;
    uint errors;
    uint8_t podToPress;
    State state;
    PhysioPodControl* control;

    void updatePodToPress();
    void onError(uint8_t pod);
    void onSuccess(uint8_t pod);
    void onPodPressed(uint8_t id) override;

public:
    void initialize(long minInterval, long maxInterval, uint8_t numberOfTries); 
    void start();
    void stop();
    void update();
    void reset();
    String* returnScore();
    const char* getName() override { return "FastPress"; }

    FastPressMode(PhysioPodControl* control);
    virtual ~FastPressMode() {}
};