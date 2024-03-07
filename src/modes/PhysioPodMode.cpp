#include "PhysioPodMode.h"

PhysioPodMode* PhysioPodMode::currentMode = nullptr;

void PhysioPodMode::stop() {
    #ifdef isDebug
    Serial.println("Stopping mode...");
    #endif
    currentMode = nullptr;
}

void PhysioPodMode::start() {
    #ifdef isDebug
    Serial.println("Starting mode...");
    #endif
    currentMode = this;
}
