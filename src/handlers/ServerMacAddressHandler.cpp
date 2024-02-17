#include "isDebug.h"
#include "ServerMacAddressHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
/*
    * This is a request handler to get the server's mac address
*/
ServerMacAddressHandler::ServerMacAddressHandler() {
}

bool ServerMacAddressHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/serverMacAddress") {
        #ifdef isDebug
        Serial.println("ServerMacAddressHandler request !");
        #endif
        return true;
    }
    return false;
}

void ServerMacAddressHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    response->print(WiFi.macAddress());
    request->send(response);
}
