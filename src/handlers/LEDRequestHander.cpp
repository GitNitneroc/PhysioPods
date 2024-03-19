#ifndef LEDRequestHandler_h
#define LEDRequestHandler_h
#include "isDebug.h"
#include "ESPAsyncWebServer.h"
#include "LEDRequestHandler.h"
#include "Messages.h"
#include <esp_now.h>

LEDRequestHandler::LEDRequestHandler(void (*setPodLightState)(uint8_t, bool)) {
    this->setPodLightState = setPodLightState;
}

bool LEDRequestHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/LED") {
        #ifdef isDebug
        Serial.println("LEDRequestHandler received params: ");
        for (uint8_t i=0; i<request->params(); i++) {
            AsyncWebParameter* p = request->getParam(i);
            Serial.print(p->name());
            Serial.print(": ");
            Serial.println(p->value());
        }
        #endif
        return true;
    }
    return false;
}

void LEDRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    //this means we received a request to "/LED"
    //the parameters are always in the same ordre : LED (ON or OFF), id (the id of the pod)
    //check if the request contains a parameter "LED"
    AsyncWebParameter* ledParam = request->getParam(0);
    bool ledState;
    uint8_t destId;
    //Read the LED parameter
    if (ledParam->name()!="LED") {
        #ifdef isDebug
        Serial.println("LEDRequestHandler : error reading LED parameter, ignoring the request");
        #endif
        return;
    }else{
        if (ledParam->value()=="ON") {
            ledState = true;
        } else {
            ledState = false;
        }
    }
    //Read the id parameter
    AsyncWebParameter* idParam = request->getParam(1);
    if (idParam->name()!="id") {
        #ifdef isDebug
        Serial.println("LEDRequestHandler : error reading id parameter, ignoring the request");
        #endif
        return;
    }else{      
        destId = idParam->value().toInt();
    }

    //let the serverPod handle the request
    setPodLightState(destId, ledState);

    //send some response to the client
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("OK");
    request->send(response);
}
#endif