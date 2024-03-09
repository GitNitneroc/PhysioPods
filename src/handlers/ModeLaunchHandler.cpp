#include "isDebug.h"
#include "ModeLaunchHandler.h"
#include "ESPAsyncWebServer.h"
#include "modes/PhysioPodMode.h"
#include "modes/FastPressMode.h"

/*
    * This is a request handler to launch a mode.
*/
ModeLaunchHandler::ModeLaunchHandler(void (*startMode)(PhysioPodMode* mode), PhysioPodControl *control) {
    this->startMode = startMode;
    this->control = control;
    this->htmlSuccess = new String(
        #include "../html/modeLaunchSuccess.html"
    );
    this->htmlFail = new String(
        #include "../html/modeLaunchFail.html"
    );
}

bool ModeLaunchHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/launchMode") {      
        #ifdef isDebug
        Serial.println("ModeLaunchHandler request !");
        for (uint8_t i=0; i<request->params(); i++) {
            AsyncWebParameter* p = request->getParam(i);
            Serial.print(p->name());
            Serial.print(": ");
            Serial.println(p->value());
        }
        #endif
        return true;
    }
    return false;
}

/*
    Helper function to send the response to the client.
*/
void ModeLaunchHandler::sendResponse(AsyncWebServerRequest *request, String* html) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response);
}

/* Takes care of launching the mode, but makes sure the previous one is stopped before*/
void ModeLaunchHandler::launchNewMode(PhysioPodMode* mode) {
    //stop the current mode before starting a new one
    if (PhysioPodMode::currentMode != nullptr && PhysioPodMode::currentMode->isRunning()){
        #ifdef isDebug
        Serial.println("There is a mode under use : Stopping it !");
        #endif
        PhysioPodMode::currentMode->stop();
    }
    //start the mode
    startMode(mode);
}

void ModeLaunchHandler::handleRequest(AsyncWebServerRequest *request) {
    //check if the user wants to restart the same mode
    if (request->getParam("restart") != NULL) {
        Serial.println("User wants to restart the current mode");
        if (PhysioPodMode::currentMode != nullptr){
            #ifdef isDebug
            Serial.println("There is a mode under use : Stopping it !");
            #endif
            if (PhysioPodMode::currentMode->isRunning()){
                PhysioPodMode::currentMode->stop();
            }
            PhysioPodMode::currentMode->start();
            sendResponse(request, htmlSuccess);
        } else{
            Serial.println("No mode to restart");
            sendResponse(request, htmlFail);
        }
        return;
    }

    //check if the user wants to launch a new mode
    String modeName = request->getParam("mode")->value();
    if (modeName == "FP") {
        Serial.println("User wants to launch Fast Press mode");
        //TODO : this could be a function in the FastPressMode class, it would return a FastPressMode object if the params are correct
        AsyncWebParameter* minIntervalParam = request->getParam("minInterval");
        AsyncWebParameter* maxIntervalParam = request->getParam("maxInterval");
        AsyncWebParameter* triesParam = request->getParam("tries");
        if (minIntervalParam == NULL || maxIntervalParam == NULL || triesParam == NULL) {
            Serial.println("could not read a parameter");
            sendResponse(request, htmlFail);
            return;
        }
        //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
        //remember this is in seconds
        long minInterval = minIntervalParam->value().toInt();
        long maxInterval = minInterval+ maxIntervalParam->value().toInt();
        uint8_t tries = triesParam->value().toInt();
        #ifdef isDebug
        Serial.println("minInterval : "+ String(minInterval));
        Serial.println("maxInterval : "+ String(maxInterval));
        Serial.println("tries : "+ String(tries));
        #endif

        //create the mode
        FastPressMode* newMode = new FastPressMode(control);
        #ifdef isDebug
        Serial.println("Mode created, initializing...");
        #endif
        newMode->initialize(minInterval*1000, maxInterval*1000, tries);//this is in ms

        launchNewMode(newMode);

        sendResponse(request, htmlSuccess);
        return;
    } else if (modeName == "2") {
        //TODO: implement mode 2
        Serial.println("Mode 2 launched");
        sendResponse(request, htmlSuccess);
    } else {
        Serial.println("Mode not recognized");
        sendResponse(request, htmlFail);
    }
}