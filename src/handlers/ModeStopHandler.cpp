#include "ModeStopHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "debugPrint.h"
#include <esp_now.h>
#include "modes/PhysioPodMode.h"

/*
    * This is a request handler to get the server's mac address
*/
ModeStopHandler::ModeStopHandler() {
}

bool ModeStopHandler::canHandle(AsyncWebServerRequest *request) const{
    if (request->url()=="/modeStop") {
        #ifdef isDebug
        DebugPrintln("ModeStopHandler request !");
        #endif
        return true;
    }
    return false;
}

void ModeStopHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    if (PhysioPodMode::currentMode!=nullptr && PhysioPodMode::currentMode->isRunning()){
        PhysioPodMode::currentMode->stop();
        response->print("mode stopped");
        #ifdef isDebug
        DebugPrintln("Mode stopped !");
        #endif
    }else{
        response->print("error : no mode currently running !");
        #ifdef isDebug
        DebugPrintln("No mode running !");
        #endif
    }
    
    //send the response
    request->send(response);
}
