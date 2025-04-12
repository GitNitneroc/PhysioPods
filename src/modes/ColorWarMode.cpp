#include "ColorWarMode.h"
#include "ServerPod.h"
#include "debugPrint.h"

ColorWarParameters ColorWarMode::parameters = {0,0,0};

void ColorWarMode::generateColors() {
    for (uint8_t i = 0; i < nColors; i++) {
        uint8_t h = 255 / nColors * i;
        CRGB color;
        hsv2rgb_rainbow(CHSV(h, 255, 128), color);
        colors[i] = color;
    }
}

void ColorWarMode::resetScores() {
    for (uint8_t i = 0; i < nColors; i++) {
        scores[i] = 0;
    }
}

void ColorWarMode::resetPodsColors() {
    for (uint8_t i = 0; i < nPods; i++) {
        podsColors[i] = 0;
    }
}

void ColorWarMode::reset() {
    resetScores();
    resetPodsColors();
    StartTimer = millis();
    //turn off every pod
    #ifdef isDebug
    DebugPrintln("ColorWarMode : turning off "+String(nPods)+" pods");
    #endif
    for (uint8_t i = 0; i < nPods; i++) {
        ServerPod::setPodLightState(i, false);
    }
}

bool ColorWarMode::testRequestParameters(AsyncWebServerRequest *request) {
    const AsyncWebParameter* nColorsParam = request->getParam("nteams");
    const AsyncWebParameter* durationParam = request->getParam("duration");
    const AsyncWebParameter* probabilityParam = request->getParam("probability");
    if (nColorsParam == NULL || durationParam == NULL || probabilityParam == NULL) {
        DebugPrintln("could not read a parameter");
        return false;
    }

    uint8_t nColors = nColorsParam->value().toInt();
    uint16_t duration = durationParam->value().toInt();
    float probability = probabilityParam->value().toFloat();

    #ifdef isDebug
    DebugPrintln("nColors : "+ String(nColors));
    DebugPrintln("duration : "+ String(duration));
    DebugPrintln("probability : "+ String(probability));
    #endif

    ColorWarMode::parameters = parameters = {nColors, duration, probability};
    PhysioPodMode::modeConstructor = generateMode;
    return true;
}

PhysioPodMode* ColorWarMode::generateMode() {
    ColorWarMode* newMode = new ColorWarMode();
    #ifdef isDebug
    DebugPrintln("Mode created, initializing...");
    #endif
    newMode->initialize(parameters.nColors, parameters.duration, parameters.probability);
    return newMode;
}

void ColorWarMode::start() {
    StartTimer = millis();
    //the number of pods might have changed since the last time we played, so we resize it in start() rather than in initialize()
    //podsColors.resize(ServerPod::getInstance()->peersNum+1, 0); //this code is from when we used std::vector
    nPods = ServerPod::getInstance()->peersNum+1;
    //turn on each pod with a random color
    for (uint8_t i = 0; i < nPods; i++) {
        uint8_t randomColorId = random(0, nColors);
        CRGB randomColor = colors[randomColorId];
        podsColors[i] = randomColorId;
        ServerPod::setPodLightState(i, true, CRGB(randomColor.r, randomColor.g, randomColor.b));
    }
    //create a new score
    ScoreStorage::addScore(returnScore());
    PhysioPodMode::start();
}

void ColorWarMode::initialize(uint8_t nColors, uint16_t duration, float probability) {
    this->nColors = nColors;
    this->duration = duration;
    this->probability = probability;
    #ifdef isDebug
    DebugPrintln("ColorWarMode : initializing with " + String(nColors) + " colors, a duration of " + String(duration) + "s and a probability of " + String(probability));
    #endif
    resetScores();

    generateColors();

    resetPodsColors(); //not sure this is necessary, start does something like that anyway
    StartTimer = millis();
    #ifdef isDebug
    DebugPrintln("ColorWarMode : turning off all connected pods");
    #endif
    //we can't use nPods now, this will only be initialized in start(), so that pod number can still change between games
    for (uint8_t i = 0; i < ServerPod::peersNum+2; i++) { //+2 because peers does not count the server
        ServerPod::setPodLightState(i, false);
    }
}

void ColorWarMode::update() {
    //is it time to stop ?
    if (millis() - StartTimer > duration*1000) { //This is in ms
        #ifdef isDebug
        DebugPrintln("ColorWarMode : time's up");
        #endif
        ServerPod::setPodLightState(255, true, CRGB::Green, LightMode::PULSE_ON_OFF_LONG); //end of game
        stop();
    }

    //is it time to test score and probability ?
    if ( millis() - lastTest > 100) {
        lastTest = millis();
        //test for random change
        if (random(0,10000) < probability * 10000) {
            //select a random pod and a random color
            uint8_t podId = random(0, nPods);
            uint8_t rancomColorId = random(0, nColors);
            CRGB newColor = colors[rancomColorId];
            podsColors[podId] = rancomColorId;
            #ifdef isDebug
            DebugPrintln("ColorWarMode : random change to color n"+String(rancomColorId)+" for pod n"+String(podId));
            #endif
            ServerPod::setPodLightState(podId, true, CRGB(newColor.r, newColor.g, newColor.b));
        }
        
        //compute the scores :
        for (uint8_t i = 0; i < nPods; i++) {
            scores[podsColors[i]]++;
        }
    }
}

void ColorWarMode::stop() {
    //update the score one last time
    ScoreStorage::updateScore(returnScore());
    PhysioPodMode::stop();
}

String* ColorWarMode::returnScore() {
    //Convert to percentages :
    int total = 0;
    for (uint8_t i = 0; i < nColors; i++) {
        total += scores[i];
    }
    //create the string
    String* score =  new String("{\"mode\": \"ColorWar\", \"duration\": " + String((int)(millis() - StartTimer)/1000) + ", \"scores\": [");
   
    for (uint8_t i = 0; i < nColors; i++) {
        char buffer[6];  // Buffer to hold the decimal string
        float percentage = static_cast<float>(scores[i]) * 100.0f / total;
        dtostrf(percentage, 4, 2, buffer);  // Convert to string with 2 decimal places
        *score += buffer;  // Append to the score string
        if (i < nColors - 1) {
            *score += ", ";
        }
    }
    *score += "]}";
    return score;
    //{"mode": "ColorWar", "duration": 60, "scores": [70.65, 29.35]}
}

void ColorWarMode::onPodPressed(uint8_t id) {
    //cycle the color
    podsColors[id] = (podsColors[id] + 1) % nColors;
    ServerPod::setPodLightState(id, true, CRGB(colors[podsColors[id]].r, colors[podsColors[id]].g, colors[podsColors[id]].b));
}

ColorWarMode::ColorWarMode() {
    StartTimer = 0;
    lastTest = 0;
    probability = 0;
    //fill the arrays with 0
    std::fill_n(scores, 255, 0);
    std::fill_n(podsColors, 255, 0);
    //duration, nColors and probability are set in initialize() 
}

/* ColorWarMode::~ColorWarMode() {
    //delete the vectors
    colors.clear();
    scores.clear();
    podsColors.clear();
} */