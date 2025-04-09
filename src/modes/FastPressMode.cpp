#include "FastPressMode.h"
#include "controls/PhysioPodControl.h"
#include "esp_now.h"
#include "ServerPod.h"
#include "debugPrint.h"

#define DECOY_PROBABILITY 5 // 1/5 chance of having a decoy

//TODO : re-tester les decoys.
//TODO : donner des stats par pods ? Genre si pod 1 = main gauche ça me tente de savoir que c'est plus lent

void FastPressMode::initialize(long minInterval, long maxInterval, bool timeLimit, uint16_t limit, bool useDecoy, bool avoidRepeat, uint8_t nColors) {
    this->minInterval = minInterval;
    this->maxInterval = maxInterval;
    this->timeLimit = timeLimit;
    this->limit = limit;
    this->useDecoy = useDecoy;
    this->nColors = nColors;
    this->avoidRepeat = avoidRepeat;

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

FastPressModeParameters FastPressMode::parameters = {0,0,0,0,0,0,0};

bool FastPressMode::testRequestParameters(AsyncWebServerRequest *request) {
    
    const AsyncWebParameter* minIntervalParam = request->getParam("minInterval");
    const AsyncWebParameter* maxIntervalParam = request->getParam("maxInterval");
    const AsyncWebParameter* limitParam = request->getParam("limit"); //number of seconds or number of tries
    const AsyncWebParameter* limitationParam = request->getParam("limitation"); //0 for time, 1 for tries
    const AsyncWebParameter* useDecoyParam = request->getParam("decoy"); //optional traps or not
    const AsyncWebParameter* avoidRepeatParam = request->getParam("avoidRepeat"); //optional avoid repeat or not
    const AsyncWebParameter* nColorsParam = request->getParam("fastPressNColors");

    if (minIntervalParam == NULL || maxIntervalParam == NULL || limitParam == NULL || nColorsParam == NULL || limitationParam == NULL){
        DebugPrintln("could not read a parameter");
        //NB : the decoy parameter is optional
        return false;
    }
    //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
    //remember this is in 10th of seconds
    long minInterval = minIntervalParam->value().toInt(); //This doesn't check for 0 minterval, which could be problematic but should be client-side validation
    long maxInterval = minInterval+ maxIntervalParam->value().toInt();
    bool timeLimit = limitationParam->value().equals("0"); //0 for time, 1 for tries
    uint8_t limit = limitParam->value().toInt();
    uint8_t nColors = nColorsParam->value().toInt();
    bool useDecoy = false; //default value for decoy
    if (useDecoyParam != NULL){
        useDecoy = useDecoyParam->value().equals("1");
    }
    bool avoidRepeat = false; //default value for avoidRepeat
    if (avoidRepeatParam != NULL){
        avoidRepeat = avoidRepeatParam->value().equals("1");
    }
    //check the parameters

    #ifdef isDebug
    DebugPrintln("minInterval : "+ String(minInterval)+" tenth of sec");
    DebugPrintln("maxInterval : "+ String(maxInterval)+" tenth of sec");
    DebugPrintln(timeLimit?"timeLimited":"tryLimited");
    DebugPrintln("limit : "+ String(limit));
    DebugPrintln("useDecoy : "+ String(useDecoy));
    DebugPrintln("avoidRepeat : "+ String(avoidRepeat));
    DebugPrintln("nColors : "+ String(nColors));
    #endif

    FastPressMode::parameters = {minInterval, maxInterval, limit, useDecoy, avoidRepeat, nColors, timeLimit};

    PhysioPodMode::modeConstructor = generateMode;

    return true;
}

PhysioPodMode* FastPressMode::generateMode() {
    FastPressMode* newMode = new FastPressMode();
    #ifdef isDebug
    DebugPrintln("Mode created, initializing...");
    #endif
    newMode->initialize(FastPressMode::parameters.minInterval*100, FastPressMode::parameters.maxInterval*100, FastPressMode::parameters.timeLimit, FastPressMode::parameters.limit, FastPressMode::parameters.useDecoy, FastPressMode::parameters.avoidRepeat, FastPressMode::parameters.nColors);//this is in ms
    return newMode;
}

void FastPressMode::onPodPressed(uint8_t id){
    #ifdef isDebug
    DebugPrintln("FastPressMode : pod "+String(id)+" pressed");
    #endif

    switch (state){
    case WAIT_FOR_PRESS:{
        #ifdef isDebug
        DebugPrintln("Press happening during state : wait for press");
        #endif
        if (id == podToPress){
            #ifdef isDebug
            DebugPrintln("Correct pod pressed !");
            #endif
            ServerPod::setPodLightState(podToPress,false);
            onSuccess(id);
        } else {
            #ifdef isDebug
            DebugPrintln("Wrong pod pressed !");
            #endif
            //the user pressed the wrong pod
            onError(id);
        }
        break;
    }
    case DURING_INTERVAL:{
        #ifdef isDebug
        DebugPrintln("Press happening during interval");
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
    lastPodPressed = 255; //no last pod pressed
    interval = 0;
    timer = 0;
    timeLimit = false;
    limit = 0;
}

void FastPressMode::stop() {
    state = STOPPED;
    #ifdef isDebug
    DebugPrintln("FastPressMode stopped");
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
    DebugPrintln("Success ! score : "+String(score));
    #endif
    if (currentTry < limit){ //TODO : this is not the only condition to stop the game
        updatePodToPress();
    } else {
        #ifdef isDebug
        DebugPrintln("FastPressMode : game over after "+String(currentTry)+" limit !");
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
    //are we time limited and over the limit ?
    if(timeLimit && millis()-startTimer >= limit*1000){
        if (state != STOPPED){
            #ifdef isDebug
            DebugPrintln("FastPressMode : game over after "+String(limit)+" seconds !");
            #endif
            ServerPod::setPodLightState(255,true, CRGB::Green, LightMode::PULSE_ON_OFF_LONG);
            //little hack to let the time to display this flash
            state = STOPPED;
        }else{
            //give 0.5 more seconds to display the end of game
            if (millis()-startTimer >= (limit+0.5)*1000){
                stop();
            }
        }
    }

    switch (state){
        case STOPPED:{
            //do nothing
            break;
        }
        case DURING_INTERVAL:{
            if (millis()- timer >= interval){
                #ifdef isDebug
                DebugPrintln("FastPressMode interval over");
                #endif
                //This is different if this is a decoy or not
                if (isDecoy && !decoyIsLit){ //decoy is Lit ne sert en fait à rien on pourrait mettre isDecoy à false après avoir défini un nouveau pod et un nouvel interval
                    #ifdef isDebug
                    DebugPrintln("Decoy pod lit");
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
                    DebugPrintln("target pod lit in color : "+String(randomColorIndex));
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
    startTimer = 0;
    lastPodPressed = 255; //no last pod pressed
}

/*
    This function starts a new interval and sets the pod to press.
    It switches the state to DURING_INTERVAL
*/
void FastPressMode::updatePodToPress() {
    #ifdef isDebug
    DebugPrintln("Updating pod to press");
    #endif
    //choose if this is a decoy
    if (useDecoy && random(0,DECOY_PROBABILITY) == 0){
        isDecoy = true;
    } else {
        isDecoy = false;
    }
    //choose the pod to press :
    if (avoidRepeat && ServerPod::getInstance()->peersNum > 0 && lastPodPressed != 255){
        //we should avoid the last pod pressed, there are at least 2 pods, and there has been a previous pod pressed
        lastPodPressed = podToPress; //save the last pod pressed
        podToPress = random(ServerPod::getInstance()->peersNum); //number of peers + 1 for the server, -1 for the last pod pressed
        if (podToPress >= lastPodPressed){
            podToPress++;
        }
    }else{
        //choose any pod
        lastPodPressed = podToPress; //save the last pod pressed
        podToPress = random(ServerPod::getInstance()->peersNum+1);//number of peers + 1
    }
    //if we are using decoys, we should not use the same pod as the decoy
    interval = random(minInterval, maxInterval);
    timer = millis();
    state = DURING_INTERVAL;
    #ifdef isDebug //TODO : this is not the only end of game condition
    DebugPrintf("Try %d/%d ! Pod to press : %d... in %d ms !\n",currentTry, limit, podToPress, interval);
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

    startTimer = millis();

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
    int tries = currentTry<=limit?currentTry:limit;

    //TODO : tries n'est plus le seul critère de fin de partie, il faudrait le renommer
    sprintf(scoreChar, "{\"mode\": \"FastPress\", \"tries\": %d, \"score\": %d, \"errors\": %d, \"meanDelay\": %d}", tries, this->score, this->errors, meanDelay);

    String* scoreStr = new String(scoreChar);
    return scoreStr;
}