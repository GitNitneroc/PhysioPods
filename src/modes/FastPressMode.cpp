#include "isDebug.h"
#include "FastPressMode.h"
#include "controls/PhysioPodControl.h"
#include "../pins.h"

void FastPressMode::initialize(long minInterval, long maxInterval, uint8_t numberOfTries) {
    this->minInterval = minInterval;
    this->maxInterval = maxInterval;
    this->numberOfTries = numberOfTries;
    reset();
}

FastPressMode::FastPressMode(PhysioPodControl* control) {
    this->control = control;
}

void FastPressMode::stop() {
    state = STOPPED;
    //let's clean things up
    reset();
}

void FastPressMode::lightPod(uint pod) {
    //TODO : light the pod
    digitalWrite(LED_PIN, HIGH);
}

void FastPressMode::unlightPod(uint pod) {
    //TODO : unlight the pod
    digitalWrite(LED_PIN, LOW);
}

void FastPressMode::onError(uint pod) {
    //TODO : display an error on all pods or something like that ?
    errors++;
    score--;
}

//TODO tout ça est à refaire maintenant

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
            if (millis()- timer < interval){
                //we are still in the interval
                bool pressed = control->checkControl();
                if (pressed) {
                    //the user pressed the button too early
                    onError(podToPress);
                    ScoreStorage::updateScore(returnScore());
                    state = WAIT_FOR_RELEASE;
                }
            }else{
                //the interval is over
                #ifdef isDebug
                Serial.println("FastPressMode interval over");
                #endif
                //the interval is over, we can now light the pod and wait for the user to press it
                state = WAIT_FOR_PRESS;
                this->lightPod(podToPress);
                timer = millis();
            }
            break;
        }

        case WAIT_FOR_PRESS:{
            //we are waiting for the user to press the button
            //check control
            bool pressed = control->checkControl();
            if (pressed) {
                //the user pressed the button
                score++;
                pressDelay += millis() - timer;
                currentTry++;
                ScoreStorage::updateScore(returnScore());
                state = WAIT_FOR_RELEASE;
                this->unlightPod(podToPress);
            }
            break;
        }
        
        case WAIT_FOR_RELEASE:{
            //we are waiting for the user to release the button, so we can start a new interval
            //TODO : this might be stupid, the user could pause the game by keeping the button pressed
            bool pressed = control->checkControl();
            if (!pressed) {
                updatePodToPress();
            }
            
            if (currentTry >= numberOfTries) {
                //the game is over
                #ifdef isDebug
                Serial.println("FastPressMode game over");
                #endif
                stop();
            }
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
    podToPress = 0; //TODO : randomize the pod to press
    interval = random(minInterval, maxInterval);
    timer = millis();
    state = DURING_INTERVAL;
    #ifdef isDebug
    Serial.print(timer);
    Serial.print(" - ");
    Serial.print(minInterval);
    Serial.print(" - ");
    Serial.print(interval);
    Serial.print(" - ");
    Serial.print(maxInterval);
    Serial.println(" !");
    Serial.println("FastPressMode interval started");
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