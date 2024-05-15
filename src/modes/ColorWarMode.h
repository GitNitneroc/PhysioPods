#include "PhysioPodMode.h"
#include "ESPAsyncWebServer.h"
#include <FastLED.h>

struct ColorWarParameters{
    uint8_t nColors;
    uint16_t duration;
    float probability;
};

class ColorWarMode : public PhysioPodMode {

protected :
    uint8_t nColors;//the number of colors in the game
    uint8_t nPods;//number of pods for this game
    //this is initialized with the biggest possible number of pods/colors (ie teams)
    uint8_t podsColors[255];//the colors of each pod
    long scores[255];//the scores of each color
    CRGB colors[255];//the rgb codes of each color in the game
    uint16_t duration;//the duration of the game
    unsigned long StartTimer;
    unsigned long lastTest;
    float probability;

    void generateColors();
    void resetScores();
    void resetPodsColors();
    static ColorWarParameters parameters;

public:
    void initialize(uint8_t nColors, uint16_t duration, float probability); //We could add team names ?
    
    static bool testRequestParameters(AsyncWebServerRequest *request);
    static PhysioPodMode* generateMode();

    const char* getName() override { return "ColorWarMode"; }

    void start() override;
    void stop() override;
    void update() override;
    void reset() override;
    String* returnScore() override;
    void onPodPressed(uint8_t id) override;

    //static Color hslToColor(uint16_t h, uint8_t s, uint8_t l);

    ColorWarMode();
};