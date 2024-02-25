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
        Serial.println("No mac address provided");
        #endif
    }else{
        #ifdef isDebug
        Serial.println("Client mac address : "+clientMac->value());
        #endif
        //TODO add the client mac address to the list of known mac addresses and send back an id or something
        response->print("\r\n1"); //for now we just send back a id of 1
    }

    request->send(response);
}
