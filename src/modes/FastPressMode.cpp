#include "FastPressMode.h"
#include "controls/PhysioPodControl.h"
#include "esp_now.h"
#include "ServerPod.h"

#define DECOY_PROBABILITY 5 // 1/5 chance of having a decoy

//TODO : re-tester les decoys.
//TODO : donner des stats par pods ? Genre si pod 1 = main gauche ça me tente de savoir que c'est plus lent

void FastPressMode::initialize(long minInterval, long maxInterval, uint8_t numberOfTries, bool useDecoy, uint8_t nColors) {
    this->minInterval = minInterval;
    this->maxInterval = maxInterval;
    this->numberOfTries = numberOfTries;
    this->useDecoy = useDecoy;
    this->nColors = nColors;

    this->generateColors(); //create the colors array ([0] is the error color)

    reset();
}

void FastPressMode::generateColors() {
    //note : the error color is always red, so the min nColors is 1, there are in fact 2 colors, so we are using nColors+1
    for (uint8_t i = 0; i < nColors+1; i++) {
        uint8_t h = 255 / (nColors+1) * i;
        CRGB color;
        hsv2rgb_rainbow(CHSV(h, 255, 128), color);
        colors[i] = color;
    }
}

FastPressModeParameters FastPressMode::parameters = {0,0,0,0,0};

bool FastPressMode::testRequestParameters(AsyncWebServerRequest *request) {
    
    AsyncWebParameter* minIntervalParam = request->getParam("minInterval");
    AsyncWebParameter* maxIntervalParam = request->getParam("maxInterval");
    AsyncWebParameter* triesParam = request->getParam("tries");
    AsyncWebParameter* useDecoyParam = request->getParam("decoy");
    AsyncWebParameter* nColorsParam = request->getParam("fastPressNColors");

    if (minIntervalParam == NULL || maxIntervalParam == NULL || triesParam == NULL || nColorsParam == NULL){
        Serial.println("could not read a parameter");
        //NB : the decoy parameter is optional
        return false;
    }
    //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
    //remember this is in 10th of seconds
    long minInterval = minIntervalParam->value().toInt(); //This doesn't check for 0 minterval, which could be problematic but should be client-side validation
    long maxInterval = minInterval+ maxIntervalParam->value().toInt();
    uint8_t tries = triesParam->value().toInt();
    uint8_t nColors = nColorsParam->value().toInt();
    bool useDecoy = false; //default value for decoy
    if (useDecoyParam != NULL){
        useDecoy = useDecoyParam->value().equals("1");
    }

    #ifdef isDebug
    Serial.println("minInterval : "+ String(minInterval)+" tenth of sec");
    Serial.println("maxInterval : "+ String(maxInterval)+" tenth of sec");
    Serial.println("tries : "+ String(tries));
    Serial.println("useDecoy : "+ String(useDecoy));
    Serial.println("nColors : "+ String(nColors));
    #endif

    FastPressMode::parameters = {minInterval, maxInterval, tries, useDecoy, nColors};

    PhysioPodMode::modeConstructor = generateMode;

    return true;
}

PhysioPodMode* FastPressMode::generateMode() {
    FastPressMode* newMode = new FastPressMode();
    #ifdef isDebug
    Serial.println("Mode created, initializing...");
    #endif
    newMode->initialize(FastPressMode::parameters.minInterval*100, FastPressMode::parameters.maxInterval*100, FastPressMode::parameters.numberOfTries, FastPressMode::parameters.useDecoy, FastPressMode::parameters.nColors);//this is in ms
    return newMode;
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

FastPressMode::FastPressMode() {
    state = STOPPED;
    score = 0;
    errors = 0;
    pressDelay = 0;
    currentTry = 0;
    podToPress = 0;
    interval = 0;
    timer = 0;
    numberOfTries = 0;
}

void FastPressMode::stop() {
    state = STOPPED;
    #ifdef isDebug
    Serial.println("FastPressMode stopped");
    #endif
    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //update the score one last time
    ScoreStorage::updateScore(returnScore());

    //call base stop
    PhysioPodMode::stop();
}

void FastPressMode::onSuccess(uint8_t pod) {
    //the user pressed the right pod
    score++;
    pressDelay += millis() - timer;
    currentTry++;
    #ifdef isDebug
    Serial.println("Success ! score : "+String(score));
    #endif
    if (currentTry < numberOfTries){
        updatePodToPress();
    } else {
        #ifdef isDebug
        Serial.println("FastPressMode : game over after "+String(currentTry)+" tries !");
        #endif
        ServerPod::setPodLightState(255,true, CRGB::Green, LightMode::PULSE_ON_OFF_LONG);
        //the game is over
        stop();
    }
}

void FastPressMode::onError(uint8_t pod) {
    //display a short error on the pod
    ServerPod::setPodLightState(pod, true, colors[0], LightMode::PULSE_ON_OFF_SHORT);
    errors++;
    score--;
}


void FastPressMode::update() {
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
                //This is different if this is a decoy or not
                if (isDecoy && !decoyIsLit){ //decoy is Lit ne sert en fait à rien on pourrait mettre isDecoy à false après avoir défini un nouveau pod et un nouvel interval
                    #ifdef isDebug
                    Serial.println("Decoy pod lit");
                    #endif
                    ServerPod::setPodLightState(podToPress,true, CRGB::Blue, LightMode::SIMPLE);
                    timer = millis(); //we reset the timer
                    //let's choose a new, real pod to press
                    podToPress = random(ServerPod::getInstance()->peersNum+1);//number of peers + 1
                    interval = random(timer, maxInterval); //between now and the max interval
                    decoyIsLit = true;
                }else{
                    //chose a random color for this time
                    uint8_t randomColorIndex = random(1,nColors+1); //we don't want the error color
                    CRGB randomColor = colors[randomColorIndex];

                    #ifdef isDebug
                    Serial.println("target pod lit in color : "+String(randomColorIndex));
                    #endif
                    
                    //the interval is over and we are not in a decoy, we should light the pod
                    ServerPod::setPodLightState(podToPress,true, randomColor, LightMode::SIMPLE);
                    timer = millis();
                    state = WAIT_FOR_PRESS;
                }
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
    //choose if this is a decoy
    if (useDecoy && random(0,DECOY_PROBABILITY) == 0){
        isDecoy = true;
    } else {
        isDecoy = false;
    }
    //choose the pod to press
    podToPress = random(ServerPod::getInstance()->peersNum+1);//number of peers + 1
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

    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //prepare the first interval
    updatePodToPress();

    //call base start
    PhysioPodMode::start();
}

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