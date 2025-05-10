#include "VisualTimerMode.h"
#include "ServerPod.h"
#include "debugPrint.h"

void VisualTimerMode::initialize(long workTime, long restTime, uint8_t numberOfCycles) {
    this->workTime = workTime;
    this->restTime = restTime;
    this->numberOfCycles = numberOfCycles;
    reset();
}

VisualTimerModeParameters VisualTimerMode::parameters = {0, 0, 0};

VisualTimerMode::VisualTimerMode() {
    timer = 0;
    restTime = 0;
    workTime = 0;
    numberOfCycles = 0;
    currentCycle = 0;
}

void VisualTimerMode::stop() {
    #ifdef isDebug
    DebugPrintln("VisualTimerMode stopped");
    #endif
    //ServerPod::setPodLightState(255, false); //turn off the pods' light
    //Make a final flash of the light to indicate the end of the session
    ServerPod::setPodLightState(255, true, CRGB(0, 255, 0), LightMode::PULSE_ON_OFF_LONG);
    PhysioPodMode::stop();
}

void VisualTimerMode::update() {
    //the idle state is used to wait for the start of the mode
    if (state == VisualTimerState::IDLE) {
        //go to the working state
        state = VisualTimerState::WORKING;
        ServerPod::setPodLightState(255, true, CRGB(255, 0, 0), LightMode::LOADING_BAR, workTime); //turn on the pod light with a red color
        ServerPod::setPodBuzzerState(255, true, 3000, 1000); //turn on the buzzer for 1 second
    }

    if (state == VisualTimerState::WORKING) {
        if (millis() - timer >= workTime*1000) {
            //switch to rest state
            state = VisualTimerState::RESTING;
            timer = millis();
            ServerPod::setPodLightState(255, false); //turn off the pods' light
            ServerPod::setPodLightState(255,true, CRGB(0, 0, 255), LightMode::UNLOADING_BAR, restTime); //turn on the pod light with a blue color

            //turn on the buzzer
            ServerPod::setPodBuzzerState(255, true, 3000, 1000); //turn on the buzzer for 1 second

            #ifdef isDebug
            DebugPrintln("Switching to rest state");
            #endif
        }//there is no else, the light updates are done by the LightTask
    } else if (state == VisualTimerState::RESTING) {
        if (millis() - timer >= restTime*1000) {
            //switch to work state
            currentCycle++;

            //turn on the buzzer
            ServerPod::setPodBuzzerState(255, true, 3000, 1000); //turn on the buzzer for 1 second

            if (currentCycle >= numberOfCycles) {
                //stop the mode
                stop();
                return;
            }
            state = VisualTimerState::WORKING;
            timer = millis();
            ServerPod::setPodLightState(255, true, CRGB(255, 0, 0), LightMode::LOADING_BAR, workTime); //turn on the pod light with a red color
            #ifdef isDebug
            DebugPrintln("Switching to work state");
            #endif
        }//there is no else, the light updates are done by the LightTask
    }
}

String* VisualTimerMode::returnScore() {
    //return score
    //empty return for now
    return new String("");
}

bool VisualTimerMode::testRequestParameters(AsyncWebServerRequest *request) {
    
    const AsyncWebParameter* workTimeParam = request->getParam("work");
    const AsyncWebParameter* restTimeParam = request->getParam("rest");
    const AsyncWebParameter* cycleParam = request->getParam("cycles");


    if (workTimeParam == NULL || restTimeParam == NULL || cycleParam == NULL) {
        DebugPrintln("could not read a parameter");
        return false;
    }
    //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
    long workTime = workTimeParam->value().toInt(); 
    long restTime = restTimeParam->value().toInt();
    uint8_t cycles = cycleParam->value().toInt();
    

    #ifdef isDebug
    DebugPrintln("workTime : "+ String(workTime)+" sec");
    DebugPrintln("restTime : "+ String(restTime)+" sec");
    DebugPrintln("cycles : "+ String(cycles));
    #endif

    VisualTimerMode::parameters = {workTime, restTime, cycles};

    PhysioPodMode::modeConstructor = generateMode;

    return true;
}

PhysioPodMode* VisualTimerMode::generateMode() {
    VisualTimerMode* newMode = new VisualTimerMode();
    #ifdef isDebug
    DebugPrintln("Mode created, initializing...");
    #endif
    newMode->initialize(VisualTimerMode::parameters.workTime, VisualTimerMode::parameters.restTime, VisualTimerMode::parameters.numberOfCycles);//this is in ms
    return newMode;
}


void VisualTimerMode::reset() {
    timer = 0;
    currentCycle = 0;
    VisualTimerState state = VisualTimerState::IDLE;
}

void VisualTimerMode::start() {
    //prepare the scores
    reset();
    timer = millis();
    state = VisualTimerState::IDLE;
    //ScoreStorage::addScore(returnScore()); //maybe we could keep this away from scores lists ?
    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //call base start
    PhysioPodMode::start();
}
