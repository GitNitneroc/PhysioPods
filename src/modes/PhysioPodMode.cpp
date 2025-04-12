#include "PhysioPodMode.h"
#include "debugPrint.h"
using namespace Messages;

PhysioPodMode* PhysioPodMode::currentMode = nullptr;
PhysioPodMode* (*PhysioPodMode::modeConstructor)() = nullptr;
PhysioPodControl *PhysioPodMode::control = nullptr;

void PhysioPodMode::setControl(PhysioPodControl *control) {
    PhysioPodMode::control = control;
}   

/* 
    * this should be called by each Mode when stopping
    * it performs the necessary cleanup
    * it also call the reset method of the mode
*/
void PhysioPodMode::stop() {
    #ifdef isDebug
    DebugPrintln("PhysioPodMode : Mode is currently stopping...");
    #endif
    /* //let's clean things up
    esp_now_unregister_recv_cb(); */

    running = false;
    reset();
}

void PhysioPodMode::start() {
    #ifdef isDebug
    DebugPrintln("Starting mode...");
    #endif
/*     //register the callback
    esp_now_register_recv_cb(this->OnDataReceived); */

    running = true;
    currentMode = this;
}
