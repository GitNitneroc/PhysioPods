#include "isDebug.h"
#include "CSSRequestHandler.h"
#include "ESPAsyncWebServer.h"

/*
    * This is a request handler to get the different modes available.
*/
CSSRequestHandler::CSSRequestHandler() {
    this->css = new String(
    #include "../html/css/lit.css"
    );
}

bool CSSRequestHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/lit.css") {
        #ifdef isDebug
        Serial.println("CSS request !");
        #endif
        return true;
    }
    return false;
}

void CSSRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/css");
    response->print(*css);
    request->send(response);
}