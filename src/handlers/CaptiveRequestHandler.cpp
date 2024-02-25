#include "isDebug.h"
#include "CaptiveRequestHandler.h"
#include "ESPAsyncWebServer.h"

/*
    * This is a request handler for the captive portal.
    * It serves the index.html file for any request
*/
CaptiveRequestHandler::CaptiveRequestHandler() {
    this->html = new String(
    #include "./html/index.html"
    );
}

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request){
    #ifdef isDebug
    Serial.print("CaptiveRequestHandler received request: ");
    Serial.println(request->url());
    #endif
    return true;
}

void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response);
}
