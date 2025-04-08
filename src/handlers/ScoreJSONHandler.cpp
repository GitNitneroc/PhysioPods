#include "ScoreJSONHandler.h"
#include "ESPAsyncWebServer.h"
#include "modes/PhysioPodMode.h"
#include "Arduino.h"
#include "debugPrint.h"

/*
    * This is a request handler to get the scores in JSON format
*/
ScoreJSONHandler::ScoreJSONHandler() {
}

bool ScoreJSONHandler::canHandle(AsyncWebServerRequest *request) const{
    if (request->url()=="/score") {
        #ifdef isDebug
        DebugPrintln("ScoreJSONHandler request !");
        #endif
        return true;
    }
    return false;
}

void ScoreJSONHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");

    //if there is a game started, ask for a score update
    if (PhysioPodMode::currentMode != nullptr){
        if (PhysioPodMode::currentMode->isRunning()){
            #ifdef isDebug
            DebugPrintln("There is a mode under use : Updating score !");
            #endif
            ScoreStorage::updateScore(PhysioPodMode::currentMode->returnScore());
        }
    }

    //return the scores
    String s = ScoreStorage::getAllJSON();
    #ifdef isDebug
        DebugPrint("Sending back scores as JSON : ");
        DebugPrintln(s);
    #endif
    response->print(s);
    request->send(response);
}
