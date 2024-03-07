#include "PhysioPodMode.h"

PhysioPodMode* PhysioPodMode::currentMode = nullptr;

void PhysioPodMode::stop() {
    currentMode = nullptr;
}

void PhysioPodMode::start() {
    currentMode = this;
}
