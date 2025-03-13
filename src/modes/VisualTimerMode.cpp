#include "VisualTimerMode.h"
#include "ServerPod.h"

void VisualTimerMode::initialize(long workTime, long restTime, uint8_t numberOfCycles) {
    this->workTime = workTime;
    this->restTime = restTime;
    this->numberOfCycles = numberOfCycles;
    reset();
}

VisualTimerModeParameters VisualTimerMode::parameters = {0,0,0,0};

bool VisualTimerMode::testRequestParameters(AsyncWebServerRequest *request) {
    
    AsyncWebParameter* workTimeParam = request->getParam("workTime");
    AsyncWebParameter* restTimeParam = request->getParam("restTime");
    AsyncWebParameter* cycleParam = request->getParam("cycles");


    if (workTimeParam == NULL || restTimeParam == NULL || cyclesParam == NULL) {
        Serial.println("could not read a parameter");
        return false;
    }
    //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
    //remember this is in 10th of seconds
    long workTime = workTime->value().toInt(); //This doesn't check for 0 minterval, which could be problematic but should be client-side validation
    long restTime = restTime->value().toInt();
    uint8_t cycles = cyclesParam->value().toInt();
    }

    #ifdef isDebug
    Serial.println("workTime : "+ String(workTime)+" tenth of sec");
    Serial.println("restTime : "+ String(restTime)+" tenth of sec");
    Serial.println("cycles : "+ String(cycles));
    #endif

    VisualTimerMode::parameters = {workTime, restTime, cycles};

    PhysioPodMode::modeConstructor = generateMode;

    return true;
}

PhysioPodMode* VisualTimerMode::generateMode() {
    VisualTimerMode* newMode = new VisualTimerMode();
    #ifdef isDebug
    Serial.println("Mode created, initializing...");
    #endif
    newMode->initialize(VisualTimerMode::parameters.workTime*100, VisualTimerMode::parameters.restTime*100, VisualTimerMode::parameters.numberOfCycles, VisualTimerMode::parameters.useDecoy);//this is in ms
    return newMode;
}

void VisualTimerMode::onPodPressed(uint8_t id){
    #ifdef isDebug
    Serial.println("VisualTimerMode : pod "+String(id)+" pressed");
    #endif

    switch (state){
    case WAIT_FOR_PRESS:{
        #ifdef isDebug
        Serial.println("Press happening during state : wait for press");
        #endif
        if (id == podToPress){
            #ifdef isDebug
            Serial.println("Correct pod pressed !");
            #endif
            ServerPod::setPodLightState(podToPress,false);
            onSuccess(id);
        }
        else {
            #ifdef isDebug
            Serial.println("Wrong pod pressed !");
            #endif
            //the user pressed the wrong pod
            onError(id);
        }
        break;
    }
    case DURING_INTERVAL:{
        #ifdef isDebug
        Serial.println("Press happening during interval");
        #endif
        //the user pressed a pod too early
        onError(id);
        break;
    }
    default:
    break;
    }
}

VisualTimerMode::VisualTimerMode() {
    state = STOPPED;
    score = 0;
    errors = 0;
    pressDelay = 0;
    currentCycle = 0;
    podToPress = 0;
    interval = 0;
    numberOfCycles = 0;
}

void VisualTimerMode::stop() {
    state = STOPPED;
    #ifdef isDebug
    Serial.println("VisualTimerMode stopped");
    #endif
    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //update the score one last time
    ScoreStorage::updateScore(returnScore());

    //call base stop
    PhysioPodMode::stop();
}


void VisualTimerMode::update() {
    switch (state){
        case STOPPED:{
            //do nothing
            break;
        }
        case DURING_INTERVAL:{
            if (millis()- timer >= interval){
                #ifdef isDebug
                Serial.println("VisualTimerMode interval over");
                #endif
                //This is different if this is a decoy or not
                if (isDecoy && !decoyIsLit){
                    #ifdef isDebug
                    Serial.println("Decoy pod lit");
                    #endif
                    ServerPod::setPodLightState(podToPress,true, CRGB::Blue, LightMode::SIMPLE);
                    timer = millis(); //we reset the timer
                    //let's choose a new, real pod to press
                    podToPress = random(ServerPod::getInstance()->peersNum+1);//number of peers + 1
                    interval = random(workTime, restTime); //between now and the rest time
                    decoyIsLit = true;
                }else{
                    #ifdef isDebug
                    Serial.println("target pod lit");
                    #endif
                    //the interval is over and we are not in a decoy, we should light the pod
                    ServerPod::setPodLightState(podToPress,true, CRGB::Green, LightMode::SIMPLE);
                    timer = millis();
                    state = WAIT_FOR_PRESS;
                }
            };
            break;
        }
        case WAIT_FOR_PRESS:{
            //we are waiting for the user to press the button
            break;
        }
        case WAIT_FOR_RELEASE:{
            //this does not exist anymore
            break;
        }
    }
}

void VisualTimerMode::reset() {
    timer = 0;
    score = 0;
    errors = 0;
    pressDelay = 0;
    currentCycle = 0;
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
