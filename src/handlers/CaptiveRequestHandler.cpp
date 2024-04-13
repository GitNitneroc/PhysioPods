#include "isDebug.h"
#include "CaptiveRequestHandler.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

/*
    * This is a request handler for the captive portal.
    * It serves the index.html file for any request
*/
CaptiveRequestHandler::CaptiveRequestHandler() {
}

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request){
    #ifdef isDebug
    Serial.print("CaptiveRequestHandler received request: ");
    Serial.println(request->url());
    #endif
    return true;
}

void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/www/captivePortal.html", String(), false);
    /* AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response); */
}
