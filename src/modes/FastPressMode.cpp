#include "isDebug.h"
#include "FastPressMode.h"
#include "controls/PhysioPodControl.h"
#include "../pins.h"
#include "esp_now.h"
#include "ServerPod.h"

FastPressMode* FastPressMode::instance = nullptr;

void FastPressMode::initialize(long minInterval, long maxInterval, uint8_t numberOfTries) {
    this->minInterval = minInterval;
    this->maxInterval = maxInterval;
    this->numberOfTries = numberOfTries;
    reset();
}

//TODO : this could probably be moved to the PhysioPodMode class
void FastPressMode::OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len){
    switch (len){
    case sizeof(ControlMessage):{
        ControlMessage* message = (ControlMessage*)data;
        #ifdef isDebug
        Serial.println("Received a control message from pod "+String(message->id));
        #endif
        instance->onPodPressed(message->id);
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

void FastPressMode::onPodPressed(uint8_t id){
    #ifdef isDebug
    Serial.println("FastPressMode : pod "+String(id)+" pressed");
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
        } else {
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
        Serial.println("Press happening during state : during interval");
        #endif
        //the user pressed a pod too early
        onError(id);
        break;
    }
    default:
        break;
    }
}

FastPressMode::FastPressMode(PhysioPodControl* control) {
    this->control = control;
    instance = this;
}

void FastPressMode::stop() {
    state = STOPPED;
    //let's clean things up
    reset();
    esp_now_unregister_recv_cb();
}

void FastPressMode::onSuccess(uint8_t pod) {
    //the user pressed the right pod
    score++;
    pressDelay += millis() - timer;
    currentTry++;
    if (currentTry < numberOfTries){
        updatePodToPress();
    } else {
        //the game is over
        stop();
    }
}

void FastPressMode::onError(uint8_t pod) {
    //TODO : display an error on all pods or something like that ?
    errors++;
    score--;
}

void FastPressMode::update() {
    //TODO : check if the user pressed the wrong pod
    //TODO : check if the user took too long to press the pod
    //TODO : count the number of tries, and stop the game after a certain number of tries
    switch (state){
        case STOPPED:{
            //do nothing
            break;
        }
        case DURING_INTERVAL:{
            if (millis()- timer >= interval){
                #ifdef isDebug
                Serial.println("FastPressMode interval over");
                #endif
                //the interval is over, we should light the pod
                ServerPod::setPodLightState(podToPress,true);
                timer = millis();
                state = WAIT_FOR_PRESS;
            }
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

void FastPressMode::reset() {
    timer = 0;
    score = 0;
    errors = 0;
    pressDelay = 0;
    currentTry = 0;
}

/*
    This function starts a new interval and sets the pod to press.
    It switches the state to DURING_INTERVAL
*/
void FastPressMode::updatePodToPress() {
    #ifdef isDebug
    Serial.println("Updating pod to press");
    #endif
    podToPress = random(2); //TODO : get the number of pods from somewhere
    interval = random(minInterval, maxInterval);
    timer = millis();
    state = DURING_INTERVAL;
    #ifdef isDebug
    Serial.printf("Try %d/%d ! Pod to press : %d... in %d ms !\n",currentTry, numberOfTries, podToPress, interval);
    #endif
}

/*
    The mode should now be ready to start, and will create a new score that will be updated later
    it will therefore reset the score first
*/
void FastPressMode::start() {
    //prepare the scores
    reset();
    ScoreStorage::addScore(returnScore());

    //register the callback
    esp_now_register_recv_cb(this->OnDataReceived);

    //prepare the first interval
    updatePodToPress();
}

//TODO : this is called at every score update, it could be called at the end of the game only, to save some resources.
/*
    This function returns a JSON string with the score
*/
String* FastPressMode::returnScore() {
    char scoreChar[200]; // HACK : this should be enough
    int meanDelay = this->currentTry == 0 ? 0 : this->pressDelay / this->currentTry;
    int tries = currentTry<=numberOfTries?currentTry:numberOfTries;

    sprintf(scoreChar, "{\"mode\": \"FastPress\", \"tries\": %d, \"score\": %d, \"errors\": %d, \"meanDelay\": %d}", tries, this->score, this->errors, meanDelay);

    String* scoreStr = new String(scoreChar);
    return scoreStr;
}