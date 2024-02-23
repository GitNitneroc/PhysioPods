#include "isDebug.h"
#include "ESPAsyncWebServer.h"
#include "LEDRequestHandler.h"


//TODO : there is no reason to pass the pin of the led, it should be defined somewhere else and included
LEDRequestHandler::LEDRequestHandler(uint8_t ledPin, String* html) {
    this->ledPin = ledPin;
    this->html = html;
}

bool LEDRequestHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/") {
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
    //this means we received a request to "/"
    for (uint8_t i=0; i<request->params(); i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->name()=="LED") {
            if (p->value()=="ON") {
                digitalWrite(ledPin, HIGH);
            } else {
                digitalWrite(ledPin, LOW);
            }
            break;
        }
    }
    //send the same page back
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response);
}