#include "ColorWarMode.h"
#include "ServerPod.h"

uint8_t hue2rgb(uint8_t p, uint8_t q, int32_t t) {
    if (t < 0) t += 65536;
    if (t > 65536) t -= 65536;
    if (t < 10923) return p + (q - p) * t / 10923;
    if (t < 32768) return q;
    if (t < 54613) return p + (q - p) * (54613 - t) / 10923;
    return p;
}

Color ColorWarMode::hslToColor(uint16_t h, uint8_t s, uint8_t l) {
    uint8_t r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        uint8_t q = l < 128 ? l * (256 + s) / 256 : l + s - l * s / 256;
        uint8_t p = 2 * l - q;
        r = hue2rgb(p, q, h + 21845);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 21845);
    }

    Color rgb = { r, g, b };
    //Serial.println("rgb(" + String(rgb.r) + ", " + String(rgb.g) + ", " + String(rgb.b)+")");
    return rgb;
}

void ColorWarMode::generateColors() {
    if (colors.size() != 0){
        colors.resize(0);
    }
    for (uint8_t i = 0; i < nColors; i++) {
        uint16_t h = 65535 / nColors * i;
        colors.push_back(hslToColor(h, 255, 122));
    }
}

void ColorWarMode::resetScores() {
    for (uint8_t i = 0; i < nColors; i++) {
        scores[i] = 0;
    }
}

void ColorWarMode::resetPodsColors() {
    for (uint8_t i = 0; i < podsColors.size(); i++) {
        podsColors[i] = 0;
    }
}

void ColorWarMode::reset() {
    resetScores();
    resetPodsColors();
    StartTimer = millis();
}

void ColorWarMode::start() {
    StartTimer = millis();
    //the number of pods might have changed since the last time we played, so we resize it in start() rather than in initialize()
    podsColors.resize(ServerPod::getInstance()->peersNum+1, 0);
    //turn on each pod with a random color
    for (uint8_t i = 0; i < podsColors.size(); i++) {
        Color randomColor = colors[random(0, nColors)];
        ServerPod::setPodLightState(i, true, randomColor.r, randomColor.g, randomColor.b);
    }
    PhysioPodMode::start();
}

void ColorWarMode::initialize(uint8_t nColors, uint16_t duration, float probability) {
    this->nColors = nColors;
    this->duration = duration;
    this->probability = probability;
    #ifdef isDebug
    Serial.println("ColorWarMode : initializing with " + String(nColors) + " colors, a duration of " + String(duration) + "s and a probability of " + String(probability));
    #endif
    scores.resize(nColors, 0);

    generateColors();
    /* //debug : print the colors
    for (uint8_t i = 0; i < nColors; i++) {
        Serial.println("rgb(" + String(colors[i].r) + ", " + String(colors[i].g) + ", " + String(colors[i].b)+")");
    } */
    reset();
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
            uint8_t podId = (0, podsColors.size());
            uint8_t rancomColorId = random(0, nColors);
            Color newColor = colors[rancomColorId];
            podsColors[podId] = rancomColorId;
            #ifdef isDebug
            Serial.println("ColorWarMode : random change to color n"+String(rancomColorId)+" for pod n"+String(podId));
            #endif
            ServerPod::setPodLightState(podId, true, newColor.r, newColor.g, newColor.b);
        }
        //TODO : score points !
    }
}

void ColorWarMode::stop() {
    //turn off every pod
    #ifdef isDebug
    Serial.println("ColorWarMode : turning off "+String(podsColors.size())+" pods");
    #endif
    for (uint8_t i = 0; i < podsColors.size(); i++) {
        ServerPod::setPodLightState(i, false);
    }

    PhysioPodMode::stop();
}

String* ColorWarMode::returnScore() {
    String* score = new String();
    for (uint8_t i = 0; i < nColors; i++) {
        *score += String(scores[i]) + ",";
    }
    return score;
}

void ColorWarMode::onPodPressed(uint8_t id) {
    //cycle the color
    podsColors[id] = (podsColors[id] + 1) % nColors;
    ServerPod::setPodLightState(id, true, colors[podsColors[id]].r, colors[podsColors[id]].g, colors[podsColors[id]].b);
}

ColorWarMode::ColorWarMode() {
    StartTimer = 0;
}