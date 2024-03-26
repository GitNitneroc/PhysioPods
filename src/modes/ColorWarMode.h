#include "PhysioPodMode.h"
#include <vector>

struct Color{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class ColorWarMode : public PhysioPodMode {
protected :
    uint8_t nColors;//the number of colors in the game
    std::vector<uint8_t> podsColors;//the colors of each pod
    std::vector<long> scores;//the scores of each color
    std::vector<Color> colors;//the rgb codes of each color in the game
    uint16_t duration;//the duration of the game
    unsigned long StartTimer;
    unsigned long lastTest;
    float probability;

    void generateColors();
    void resetScores();
    void resetPodsColors();

public:
    void initialize(uint8_t nColors, uint16_t duration, float probability); //We could add team names ?
    
    const char* getName() override { return "ColorWarMode"; }

    void start() override;
    void stop() override;
    void update() override;
    void reset() override;
    String* returnScore() override;
    void onPodPressed(uint8_t id) override;

    static Color hslToColor(uint16_t h, uint8_t s, uint8_t l);
    //static uint32_t hslToColor(uint16_t h, uint8_t s, uint8_t l);

    ColorWarMode();
    ~ColorWarMode() override;
};