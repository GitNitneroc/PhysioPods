#include "ModeLaunchHandler.h"
#include "ESPAsyncWebServer.h"

#include "modes/PhysioPodMode.h"
#include "modes/FastPressMode.h"
#include "modes/FairyLightsMode.h"
#include "modes/ColorWarMode.h"
#include "modes/ChaseMode.h"
#include "modes/VisualTimerMode.h"

#include "debugPrint.h"
#include "SPIFFS.h"

/*
    * This is a request handler to launch a mode.
*/
ModeLaunchHandler::ModeLaunchHandler() {
}

bool ModeLaunchHandler::canHandle(AsyncWebServerRequest *request) const{
    if (request->url()=="/launchMode") {      
        #ifdef isDebug
        DebugPrintln("ModeLaunchHandler request !");
        for (uint8_t i=0; i<request->params(); i++) {
            const AsyncWebParameter* p = request->getParam(i);
            DebugPrint(p->name());
            DebugPrint(": ");
            DebugPrintln(p->value());
        }
        #endif
        return true;
    }
    return false;
}


void ModeLaunchHandler::sendSuccessResponse(AsyncWebServerRequest *request){
    request->send(SPIFFS, "/www/modeLaunchSuccess.html", String(), false);
}

void ModeLaunchHandler::sendFailResponse(AsyncWebServerRequest *request){
    request->send(SPIFFS, "/www/modeLaunchFail.html", String(), false);
}

void ModeLaunchHandler::handleRequest(AsyncWebServerRequest *request) {
    //check if the user wants to restart the same mode
    if (request->getParam("restart") != NULL) {
        DebugPrintln("User wants to restart the current mode");
        if (PhysioPodMode::currentMode != nullptr){
            #ifdef isDebug
            DebugPrintln("There is a mode under use : Stopping it !");
            #endif
            if (PhysioPodMode::currentMode->isRunning()){
                PhysioPodMode::currentMode->stop();
            }
            PhysioPodMode::currentMode->start();
            sendSuccessResponse(request);
        } else{
            DebugPrintln("No mode to restart");
            sendFailResponse(request);
        }
        return;
    }

    //check if the user wants to launch a new mode
    String modeName = request->getParam("mode")->value();

    bool validParams = false;
    //FASTPRESS MODE
    if (modeName == "FP") {
        DebugPrintln("User wants to launch Fast Press mode");
        DebugPrintln(PhysioPodMode::currentMode==nullptr?"No mode running":"Mode running");
        validParams = FastPressMode::testRequestParameters(request);
        if (validParams) {
            sendSuccessResponse(request);
        } else {
            sendFailResponse(request);
        }
        return;
    }else if (modeName == "CW"){
        //COLOR WAR MODE
        DebugPrintln("User wants to launch ColorWar mode");
        validParams = ColorWarMode::testRequestParameters(request);
        if (validParams) {
            sendSuccessResponse(request);
        } else {
            sendFailResponse(request);
        }
        return;
    
    } else if (modeName == "FL") {
        //FAIRY LIGHTS MODE
        DebugPrintln("User wants to launch FairyLightsMode mode");
        validParams = FairyLightsMode::testRequestParameters(request);
        if (validParams) {
            sendSuccessResponse(request);
        } else {
            sendFailResponse(request);
        }
        return;
    }else if (modeName=="Chase"){
        //CHASE MODE
        DebugPrintln("User wants to launch Chase mode");
        validParams = ChaseMode::testRequestParameters(request);
        if (validParams) {
            sendSuccessResponse(request);
        } else {
            sendFailResponse(request);
        }
        return;

    }else if (modeName=="Visual Timer"){
        //VISUAL TIMER MODE
        DebugPrintln("User wants to launch Visual Timer mode");
        validParams = VisualTimerMode::testRequestParameters(request);
        if (validParams) {
            sendSuccessResponse(request);
        } else {
            sendFailResponse(request);
        }
        return;
        
    } else {
        DebugPrintln("Mode not recognized");
        sendFailResponse(request);
    }
}
