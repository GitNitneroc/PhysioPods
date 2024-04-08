#include "isDebug.h"
#include "ModeLaunchHandler.h"
#include "ESPAsyncWebServer.h"
#include "modes/PhysioPodMode.h"
#include "modes/FastPressMode.h"
#include "modes/FairyLightsMode.h"
#include "modes/ColorWarMode.h"

/*
    * This is a request handler to launch a mode.
*/
ModeLaunchHandler::ModeLaunchHandler() {
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

    bool validParams = false;
    //FASTPRESS MODE
    if (modeName == "FP") {
        Serial.println("User wants to launch Fast Press mode");
        Serial.println(PhysioPodMode::currentMode==nullptr?"No mode running":"Mode running");
        validParams = FastPressMode::testRequestParameters(request);
        if (validParams) {
            sendResponse(request, htmlSuccess);
        } else {
            sendResponse(request, htmlFail);
        }
        return;
    }else if (modeName == "CW"){
        //COLOR WAR MODE
        Serial.println("User wants to launch ColorWar mode");
        validParams = ColorWarMode::testRequestParameters(request);
        if (validParams) {
            sendResponse(request, htmlSuccess);
        } else {
            sendResponse(request, htmlFail);
        }
        return;
    
    } else if (modeName == "FL") {
        //FAIRY LIGHTS MODE
        Serial.println("User wants to launch FairyLightsMode mode");
        validParams = FairyLightsMode::testRequestParameters(request);
        if (validParams) {
            sendResponse(request, htmlSuccess);
        } else {
            sendResponse(request, htmlFail);
        }
        return;
        
    } else {
        Serial.println("Mode not recognized");
        sendResponse(request, htmlFail);
    }
}