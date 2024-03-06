#include "PhysioPodMode.h"
#include <Arduino.h>
#include "controls/PhysioPodControl.h"
#include "scoreStorage.h"
#include "messages.h"

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
public:
    void initialize(long minInterval, long maxInterval, uint8_t numberOfTries); 
    void start();
    void stop();
    void update();
    void reset();
    String* returnScore();

    FastPressMode(PhysioPodControl* control);
    virtual ~FastPressMode() {}

private:
    long timer;
    long interval;
    long minInterval;
    long maxInterval;
    long pressDelay;
    uint numberOfTries;
    uint currentTry;
    int score;
    int errors;
    uint8_t podToPress;
    State state;
    PhysioPodControl* control;
    static FastPressMode* instance;

    void updatePodToPress();
    void onError(uint8_t pod);
    void onSuccess(uint8_t pod);
    static void OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len);
    void onPodPressed(uint8_t id) override;
};