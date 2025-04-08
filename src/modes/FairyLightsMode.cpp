#include "FairyLightsMode.h"
#include "ServerPod.h"
#include "debugPrint.h"

void FairyLightsMode::initialize(int timeByPod) {
    this->timeByPod = timeByPod;
    reset();
}

void FairyLightsMode::start() {
    running = true;
    reset();
}

void FairyLightsMode::stop() {
    running = false;
    ServerPod::setPodLightState(litPod,false);
    //update the score one last time
    ScoreStorage::updateScore(returnScore());
    reset();
}

void FairyLightsMode::reset() {
    ServerPod::setPodLightState(litPod,false);
    litPod = 0;
    timer = 0;//this ensures that the first update will change the light
}

FairyLightsParameters FairyLightsMode::parameters = {0};

bool FairyLightsMode::testRequestParameters(AsyncWebServerRequest *request) {

    const AsyncWebParameter* timeByPodParam = request->getParam("timeByPod");
    if (timeByPodParam == NULL) {
        DebugPrintln("could not read a parameter");
        return false;
    }

    int timeByPod = timeByPodParam->value().toInt();

    #ifdef isDebug
    DebugPrintln("timeByPod : "+ String(timeByPod));
    #endif

    FairyLightsMode::parameters = {timeByPod};
    PhysioPodMode::modeConstructor = generateMode;
    return true;
}

PhysioPodMode* FairyLightsMode::generateMode() {
    FairyLightsMode* newMode = new FairyLightsMode();
    #ifdef isDebug
    DebugPrintln("Mode created, initializing...");
    #endif
    newMode->initialize(FairyLightsMode::parameters.timeByPod);
    return newMode;
}

FairyLightsMode::FairyLightsMode() {
    //do nothing
}

String* FairyLightsMode::returnScore() {
    return new String("{\"mode\": \"FairyLights\"}");
}

void FairyLightsMode::onPodPressed(uint8_t id) {
    //do nothing
}

void FairyLightsMode::update() {
    if (running) {
        if (millis() - timer > timeByPod && ServerPod::getInstance()->peersNum > 0) {
            timer = millis();
            ServerPod::setPodLightState(litPod,false); //we could use 255 instead of litPod...
            litPod = (litPod + 1) % (ServerPod::getInstance()->peersNum+1);
            ServerPod::setPodLightState(litPod,true, CRGB::White);
        }
    }
}