#include "FairyLightsMode.h"
#include "ServerPod.h"

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
    reset();
}

void FairyLightsMode::reset() {
    ServerPod::setPodLightState(litPod,false);
    litPod = 0;
    timer = 0;//this ensures that the first update will change the light
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
            ServerPod::setPodLightState(litPod,true);
        }
    }
}