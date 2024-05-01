#include "ColorWarMode.h"
#include "ServerPod.h"

ColorWarParameters ColorWarMode::parameters = {0,0,0};

uint8_t hue2rgb(uint8_t p, uint8_t q, uint16_t t) {
    /*Serial.println("p : " + String(p));
    Serial.println("q : " + String(q));
    Serial.println("t : " + String(t)); */
    if (t < 10923) return p + (q - p) * t / 10923;
    if (t < 32768) return q;
    if (t < 43690) return p + (q - p) * (54613 - t) / 10923;
    return p;
}

//TODO : Il faudrait peut-être mettre cette fonction dans un fichier utils.h
Color ColorWarMode::hslToColor(uint16_t h, uint8_t s, uint8_t l) {
    Color c;

    if (s == 0) {
        c.r = c.g = c.b = l; // achromatic
    } else {
        uint8_t q = l < 128 ? l * (255 + s) / 255 : l + s- (l * s) / 255;
        /* Serial.println("q : " + String(q)); */
        uint8_t p = 2 * l - q;
        /* Serial.println("p : " + String(p)); */
        c.r = hue2rgb(p, q, h + 21845);
        c.g = hue2rgb(p, q, h);
        c.b = hue2rgb(p, q, h - 21845);
    }
    /* Serial.println("rgb(" + String(c.r) + ", " + String(c.g) + ", " + String(c.b)+")"); */
    return c;
}

void ColorWarMode::generateColors() {
    for (uint8_t i = 0; i < nColors; i++) {
        uint16_t h = 65535 / nColors * i;
        colors[i] = hslToColor(h, 255, 128);
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
    Serial.println("ColorWarMode : turning off "+String(nPods)+" pods");
    #endif
    for (uint8_t i = 0; i < nPods; i++) {
        ServerPod::setPodLightState(i, false);
    }
}

bool ColorWarMode::testRequestParameters(AsyncWebServerRequest *request) {
    AsyncWebParameter* nColorsParam = request->getParam("nteams");
    AsyncWebParameter* durationParam = request->getParam("duration");
    AsyncWebParameter* probabilityParam = request->getParam("probability");
    if (nColorsParam == NULL || durationParam == NULL || probabilityParam == NULL) {
        Serial.println("could not read a parameter");
        return false;
    }

    uint8_t nColors = nColorsParam->value().toInt();
    uint16_t duration = durationParam->value().toInt();
    float probability = probabilityParam->value().toFloat();

    #ifdef isDebug
    Serial.println("nColors : "+ String(nColors));
    Serial.println("duration : "+ String(duration));
    Serial.println("probability : "+ String(probability));
    #endif

    ColorWarMode::parameters = parameters = {nColors, duration, probability};
    PhysioPodMode::modeConstructor = generateMode;
    return true;
}

PhysioPodMode* ColorWarMode::generateMode() {
    ColorWarMode* newMode = new ColorWarMode();
    #ifdef isDebug
    Serial.println("Mode created, initializing...");
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
        Color randomColor = colors[randomColorId];
        podsColors[i] = randomColorId;
        ServerPod::setPodLightState(i, true, randomColor.r, randomColor.g, randomColor.b);
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
    Serial.println("ColorWarMode : initializing with " + String(nColors) + " colors, a duration of " + String(duration) + "s and a probability of " + String(probability));
    #endif
    resetScores();

    generateColors();

    resetPodsColors(); //not sure this is necessary, start does something like that anyway
    StartTimer = millis();
    #ifdef isDebug
    Serial.println("ColorWarMode : turning off all connected pods");
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
        Serial.println("ColorWarMode : time's up");
        #endif
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
            Color newColor = colors[rancomColorId];
            podsColors[podId] = rancomColorId;
            #ifdef isDebug
            Serial.println("ColorWarMode : random change to color n"+String(rancomColorId)+" for pod n"+String(podId));
            #endif
            ServerPod::setPodLightState(podId, true, newColor.r, newColor.g, newColor.b);
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
    ServerPod::setPodLightState(id, true, colors[podsColors[id]].r, colors[podsColors[id]].g, colors[podsColors[id]].b);
}

ColorWarMode::ColorWarMode() {
    StartTimer = 0;
    lastTest = 0;
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