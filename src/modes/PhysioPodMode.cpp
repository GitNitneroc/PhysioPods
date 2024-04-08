#include "PhysioPodMode.h"
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
    Serial.println("PhysioPodMode : Mode is currently stopping...");
    #endif
    //let's clean things up
    esp_now_unregister_recv_cb();

    running = false;
    reset();
}

void PhysioPodMode::OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len){
    switch (len){
    case sizeof(ControlMessage):{
        ControlMessage* message = (ControlMessage*)data;
        #ifdef isDebug
        Serial.println("Received a control message from pod "+String(message->id));
        #endif
        //call the current mode's press callback
        currentMode->onPodPressed(message->id);
        break;
    }
    default:
        Serial.print("Received a message of unknown length from ");
        for (int i = 0; i < 6; i++) {
            Serial.print(sender_addr[i], HEX);
            if (i<5) Serial.print(":");
        }
        Serial.println();
        break;
    }
}


void PhysioPodMode::start() {
    #ifdef isDebug
    Serial.println("Starting mode...");
    #endif
    //register the callback
    esp_now_register_recv_cb(this->OnDataReceived);

    running = true;
    currentMode = this;
}
