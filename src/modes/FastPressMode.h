#include "PhysioPodMode.h"
#include <Arduino.h>
#include "controls/PhysioPodControl.h"

enum State{
    DURING_INTERVAL,
    WAIT_FOR_PRESS,
    WAIT_FOR_RELEASE
};

/*
    This mode is a simple game where the user has to press the button as soon as possible after the LED turns on.
*/
class FastPressMode : public PhysioPodMode {
public:
    void initialize(long minInterval, long maxInterval);
    void stop();
    void update();
    void reset();
    String returnScore();

    FastPressMode(PhysioPodControl* control);
    virtual ~FastPressMode() {}

private:
    long timer;
    long interval;
    long minInterval;
    long maxInterval;
    int score;
    int errors;
    uint podToPress;
    State state;
    PhysioPodControl* control;

    void updatePodToPress();
    void lightPod(uint pod);
    void unlightPod(uint pod);
    void onError(uint pod);
};