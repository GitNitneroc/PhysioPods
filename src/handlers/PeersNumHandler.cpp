#include "PeersNumHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <esp_now.h>
#include "ServerPod.h"

PeersNumHandler::PeersNumHandler() {
}

bool PeersNumHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/peers") {
        #ifdef isDebug
        Serial.println("PeersNumHandler request !");
        #endif
        return true;
    }
    return false;
}

void PeersNumHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{\"peers\":");
    response->print(ServerPod::peersNum);
    response->print("}\n");

    /* Serial.print("{\"peers\":");
    Serial.print(ServerPod::peersNum);
    Serial.print("}\n"); */
    //send the response
    request->send(response);
}
