#include "isDebug.h"
#include "ModeChoiceHandler.h"
#include "ESPAsyncWebServer.h"

/*
    * This is a request handler to get the different modes available.
*/
ModeChoiceHandler::ModeChoiceHandler() {
    this->html = new String(
    #include "../html/modeChoice.html"
    );
}

bool ModeChoiceHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/modeChoice") {
        #ifdef isDebug
        Serial.println("ModeChoiceHandler request !");
        #endif
        return true;
    }
    return false;
}

void ModeChoiceHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response);
}