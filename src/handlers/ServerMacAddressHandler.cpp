#include "isDebug.h"
#include "ServerMacAddressHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <esp_now.h>

/*
    * This is a request handler to get the server's mac address
*/
ServerMacAddressHandler::ServerMacAddressHandler(uint8_t * peersNum) : peersNum(peersNum) {
}

bool ServerMacAddressHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/serverMacAddress") { //Remember this is hard coded in the client connection of pods too
        #ifdef isDebug
        Serial.println("ServerMacAddressHandler request !");
        #endif
        return true;
    }
    return false;
}

void ServerMacAddressHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    response->print(WiFi.macAddress());

    //add an extra line to the response if the client provided a mac address
    AsyncWebParameter* clientMac = request->getParam("mac");
    if (clientMac == NULL) {
        #ifdef isDebug
        Serial.println("This is not a pod : No mac address provided");
        #endif
    }else{
        //a new peer is connected !
        (*peersNum)++;

        #ifdef isDebug
        //read the mac address
        const char* macStr = clientMac->value().c_str();
        Serial.print("ClientPod mac address : ");
        Serial.print(macStr);
        Serial.printf(" is attributed id : %d\n", *peersNum);
        #endif
        
        //append the id to the response on a new line
        response->print("\r\n");
        response->print(*peersNum);
    }

    //send the response
    request->send(response);
}
