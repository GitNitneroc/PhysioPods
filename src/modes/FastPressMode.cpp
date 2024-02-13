#include "isDebug.h"
#include "FastPressMode.h"
#include "controls/PhysioPodControl.h"

void FastPressMode::initialize(long minInterval, long maxInterval) {
    this->minInterval = minInterval;
    this->maxInterval = maxInterval;
    score = 0;
    errors = 0;
    updatePodToPress();
    //TODO : count the number of tries, and stop the game after a certain number of tries
    //TODO : compute a reaction time
}

FastPressMode::FastPressMode(PhysioPodControl* control) {
    this->control = control;
}

void FastPressMode::stop() {
    timer = 0;
    score = 0;
}

void FastPressMode::lightPod(uint pod) {
    //TODO : light the pod
    digitalWrite(4, HIGH);
}

void FastPressMode::unlightPod(uint pod) {
    //TODO : unlight the pod
    digitalWrite(4, LOW);
}

void FastPressMode::onError(uint pod) {
    //TODO : display an error on all pods or something like that ?
    errors++;
    score--;
}

void FastPressMode::update() {
    //TODO : check if the user pressed the wrong pod
    //TODO : check if the user took too long to press the pod
    switch (state){
        case DURING_INTERVAL:
        {
            //we are waiting for the interval to be over
            if (millis() - timer > interval) {
                //the interval is over
                #ifdef isDebug
                Serial.println("FastPressMode interval over");
                #endif
                //the interval is over
                state = WAIT_FOR_PRESS;
                this->lightPod(podToPress);
                timer = millis();
            }else{
                //we are still in the interval
                bool pressed = control->checkControl();
                if (pressed) {
                    onError(podToPress);
                    state = WAIT_FOR_RELEASE;
                }
            }
            break;
        }

        case WAIT_FOR_PRESS:{

            //we are waiting for the user to press the button
            //check control
            bool pressed = control->checkControl();
            if (pressed) {
                score++;
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
            break;
        }
    }
}

void FastPressMode::reset() {
    timer = 0;
    score = 0;
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

String FastPressMode::returnScore() {
    return String(score);
}