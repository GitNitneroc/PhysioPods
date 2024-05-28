#include "ChaseMode.h"
#include "ServerPod.h"


void ChaseMode::initialize(int cycles, uint8_t* cycle, int cycleLength) {
    this->cycles = cycles;
    this->currentStep = 0;
    this->cycleLength = cycleLength;
    //copy the cycle array
    for (int i = 0; i < cycles; i++) {
        this->cycle[i] = cycle[i];
    }
    reset();
}

void ChaseMode::start() {
    running = true;
    reset();
    startTimer = millis();
    //light the first pod
    ServerPod::setPodLightState(cycle[currentStep],true, CRGB::Green, LightMode::CYCLE_FAST);
}

void ChaseMode::stop() {
    running = false;
    ServerPod::setPodLightState(this->parameters.cycle[currentStep],false);
    //update the score one last time
    ScoreStorage::updateScore(returnScore());
    reset();
}

void ChaseMode::reset() {
    ServerPod::setPodLightState(this->parameters.cycle[this->currentStep],false);
    this->currentStep = 0;
    startTimer = 0;
}

ChaseParameters ChaseMode::parameters = {0};

bool ChaseMode::testRequestParameters(AsyncWebServerRequest *request) {

    AsyncWebParameter* cyclesParam = request->getParam("cycles");
    if (cyclesParam == NULL) {
        Serial.println("could not read a parameter");
        return false;
    }
    int cycles = cyclesParam->value().toInt();

    AsyncWebParameter* cycleParam = request->getParam("cycle");
    if (cycleParam == NULL) {
        Serial.println("could not read a parameter");
        return false;
    }
    String cycleString = cycleParam->value();

    ChaseParameters parameters;
    parameters.cycles = cycles;

    //parse the cycle string
    int i = 0;
    int j = 0;
    int start = 0;
    while (i <= cycleString.length()) {
        if (i == cycleString.length() || cycleString[i] == ',') {
            parameters.cycle[j] = cycleString.substring(start, i).toInt();
            j++;
            start = i + 1;
        }
        i++;
    }
    parameters.cycleLength = j;

    #ifdef isDebug
    Serial.println("cycles : "+ String(cycles));
    Serial.println("cycle length : "+ String(parameters.cycleLength));
    Serial.println("cycle : "+ cycleString);
    #endif

    ChaseMode::parameters = parameters;
    PhysioPodMode::modeConstructor = generateMode;
    return true;
}

PhysioPodMode* ChaseMode::generateMode() {
    ChaseMode* newMode = new ChaseMode();
    #ifdef isDebug
    Serial.println("Mode created, initializing...");
    #endif
    newMode->initialize(ChaseMode::parameters.cycles, ChaseMode::parameters.cycle, ChaseMode::parameters.cycleLength);
    return newMode;
}

ChaseMode::ChaseMode() {
    startTimer = 0;
    currentStep = 0;
    currentCycle = 0;
}

String* ChaseMode::returnScore() {
    return new String("{\"mode\": \"Chase\", \"duration\": " + String((int)(millis() - startTimer)/1000)+"}");
}

void ChaseMode::onPodPressed(uint8_t id) {
    if (running) {
        if (id == cycle[currentStep]) {
            //the right pod was pressed
            currentStep++;
            ServerPod::setPodLightState(id,true, CRGB::Green, LightMode::PULSE_SHORT); //light the pod green for a short time to indicate success, it will turn off automatically
            #ifdef isDebug
            Serial.println("Pod "+String(id)+" pressed, go to step id "+String(currentStep)+" (len="+String(cycleLength)+")");
            #endif
            
            if (currentStep >= cycleLength) {
                //the last pod of cycle was pressed
                this->currentStep = 0;
                this->currentCycle++;
                #ifdef isDebug
                Serial.println("Cycle "+String(this->currentCycle)+" completed !");
                #endif
                if (this->currentCycle >= this->cycles) {
                    //the last cycle was completed
                    #ifdef isDebug
                        Serial.println("All cycles completed ! Let's stop ChaseMode");
                    #endif

                    stop();
                    return;
                }
            }
            //light the next pod
            ServerPod::setPodLightState(this->cycle[this->currentStep],true, CRGB::Teal, LightMode::CYCLE_FAST);

        } else {
            //the wrong pod was pressed
            ServerPod::setPodLightState(id,true, CRGB::Red, LightMode::PULSE_SHORT);
        }
    }
}

void ChaseMode::update() {
    //nothing to update, the mode is controlled by the pods
}