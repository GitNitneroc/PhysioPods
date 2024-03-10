#include "isDebug.h"
#include "ScoreJSONHandler.h"
#include "ESPAsyncWebServer.h"
#include "modes/PhysioPodMode.h"

/*
    * This is a request handler to get the scores in JSON format
*/
ScoreJSONHandler::ScoreJSONHandler() {
}

bool ScoreJSONHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/score") {
        #ifdef isDebug
        Serial.println("ScoreJSONHandler request !");
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
            Serial.println("There is a mode under use : Updating score !");
            #endif
            ScoreStorage::updateScore(PhysioPodMode::currentMode->returnScore());
        }
    }

    //return the scores
    response->print(ScoreStorage::getAllJSON());
    request->send(response);
}
