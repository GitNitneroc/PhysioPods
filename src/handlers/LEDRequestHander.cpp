#include "isDebug.h"
#include "ESPAsyncWebServer.h"
#include "LEDRequestHandler.h"
#include "pins.h"
#include "messages.h"
#include <esp_now.h>

LEDRequestHandler::LEDRequestHandler() {
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

    //check if the request contains a parameter "LED"
    for (uint8_t i=0; i<request->params(); i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->name()=="LED") {
            //create the message
            LEDMessage message;
            if (p->value()=="ON") {
                digitalWrite(LED_PIN, HIGH);
                message.id = 255;
                message.state = 1;
            } else {
                digitalWrite(LED_PIN, LOW);
                message.id = 255;
                message.state = 0;
            }

            //TODO : this address should be stored in memory
            uint8_t ip_addr_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

            //send the message
            esp_err_t result = esp_now_send(ip_addr_broadcast, (uint8_t *) &message, sizeof(LEDMessage));
            if (result == ESP_OK) {
                #ifdef isDebug
                Serial.println("ESP-NOW Message sent");
                #endif
            } else {
                #ifdef isDebug
                Serial.print("Error sending the ESP-NOW message : ");
                Serial.println(esp_err_to_name(result));
                #endif
            }
            //stop parsing the parameters
            break;
        }
    }
    //send the same page back
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("OK");
    request->send(response);
}