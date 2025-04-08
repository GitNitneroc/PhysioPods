#include "ModeInfoHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <esp_now.h>
#include "modes/PhysioPodMode.h"

/*
    * This is a request handler to get the info about the current mode (ie : is it running, and which one is it)
*/
ModeInfoHandler::ModeInfoHandler() {
}

bool ModeInfoHandler::canHandle(AsyncWebServerRequest *request) const{
    if (request->url()=="/modeInfo") {
        #ifdef isDebug
        Serial.println("ModeInfoHandler request !");
        #endif
        return true;
    }
    return false;
}

void ModeInfoHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    if (PhysioPodMode::currentMode!=nullptr){
        response->print("{\"mode\":\"");
        response->print(PhysioPodMode::currentMode->getName());
        response->print("\", \"isRunning\":");
        response->print(PhysioPodMode::currentMode->isRunning() ? "true" : "false");
        response->print("}");
        //for debug, print to console the same thing :
        #ifdef isDebug
        Serial.print("{\"mode\":\"");
        Serial.print(PhysioPodMode::currentMode->getName());
        Serial.print("\", \"isRunning\":");
        Serial.print(PhysioPodMode::currentMode->isRunning() ? "true" : "false");
        Serial.print("}\n");
        #endif

    }else{
        response->print("{\"mode\":null}");
    }
    
    //send the response
    request->send(response);
}
