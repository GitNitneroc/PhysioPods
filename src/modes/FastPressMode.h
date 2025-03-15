#include "PhysioPodMode.h"
#include <Arduino.h>
#include "controls/PhysioPodControl.h"
#include "scoreStorage.h"
#include "ESPAsyncWebServer.h"
#include <FastLED.h>

enum State{
    STOPPED,
    DURING_INTERVAL,
    WAIT_FOR_PRESS,
    WAIT_FOR_RELEASE
};

struct FastPressModeParameters {
    long minInterval;
    long maxInterval;
    uint8_t numberOfTries;
    bool useDecoy;
    uint8_t nColors;
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
    bool isDecoy;
    bool decoyIsLit;
    uint numberOfTries;
    uint currentTry;
    int score;
    uint errors;
    uint8_t podToPress;
    bool useDecoy;
    State state;

    CRGB colors[9]; //NOTE : the number of colors is limited to 9... this is enforced client side only for now
    uint8_t nColors; //Note this is the number of colors for a lit pod. colors[0] is always the error color (red)

    void updatePodToPress();
    void onError(uint8_t pod);
    void onSuccess(uint8_t pod);
    void onPodPressed(uint8_t id) override;
    void generateColors();

public:
    static FastPressModeParameters parameters;

    static bool testRequestParameters(AsyncWebServerRequest *request);
    static PhysioPodMode* generateMode();
    void initialize(long minInterval, long maxInterval, uint8_t numberOfTries, bool useDecoy, uint8_t nColors); 
    void start();
    void stop();
    void update();
    void reset();
    String* returnScore();
    const char* getName() override { return "FastPress"; }

    FastPressMode();
    virtual ~FastPressMode() {}
};