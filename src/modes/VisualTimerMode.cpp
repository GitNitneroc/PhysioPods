#include "VisualTimerMode.h"
#include "ServerPod.h"

void VisualTimerMode::initialize(long workTime, long restTime, uint8_t numberOfCycles) {
    this->workTime = workTime;
    this->restTime = restTime;
    this->numberOfCycles = numberOfCycles;
    reset();
}

VisualTimerModeParameters VisualTimerMode::parameters = {0, 0, 0};

bool VisualTimerMode::testRequestParameters(AsyncWebServerRequest *request) {
    
    AsyncWebParameter* workTimeParam = request->getParam("workTime");
    AsyncWebParameter* restTimeParam = request->getParam("restTime");
    AsyncWebParameter* cycleParam = request->getParam("cycles");


    if (workTimeParam == NULL || restTimeParam == NULL || cycleParam == NULL) {
        Serial.println("could not read a parameter");
        return false;
    }
    //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
    //remember this is in 10th of seconds
    long workTime = workTimeParam->value().toInt(); //This doesn't check for 0 minterval, which could be problematic but should be client-side validation
    long restTime = restTimeParam->value().toInt();
    uint8_t cycles = cycleParam->value().toInt();
    

    //#ifdef isDebug
    //Serial.println("workTime : "+ String(workTime)+" tenth of sec");
    //Serial.println("restTime : "+ String(restTime)+" tenth of sec");
    //Serial.println("cycles : "+ String(cycles));
    //#endif

    VisualTimerMode::parameters = {workTime, restTime, cycles};

    PhysioPodMode::modeConstructor = generateMode;

    return true;
}


PhysioPodMode* VisualTimerMode::generateMode() {
    VisualTimerMode* newMode = new VisualTimerMode();
    #ifdef isDebug
    Serial.println("Mode created, initializing...");
    #endif
    newMode->initialize(VisualTimerMode::parameters.workTime*100, VisualTimerMode::parameters.restTime*100, VisualTimerMode::parameters.numberOfCycles);//this is in ms
    return newMode;
}


void VisualTimerMode::reset() {
    timer = 0;
    score = 0;
    errors = 0;
    pressDelay = 0;
    //currentCycle = 0; // Comment bien le déclarer ?
    podToPress = 0;
    interval = 0;
}

void VisualTimerMode::start() {
    //prepare the scores
    reset();
    ScoreStorage::addScore(returnScore());
    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //prepare the first interval
    updatePodToPress();
    //call base start
    PhysioPodMode::start();
}
