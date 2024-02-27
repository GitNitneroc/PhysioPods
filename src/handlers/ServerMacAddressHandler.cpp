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
        //read the mac address
        const char* macStr = clientMac->value().c_str();
        #ifdef isDebug
        Serial.print("Client mac address : ");
        Serial.println(macStr);
        #endif
        
        /* For now this is disabled, we only use the broadcast address
        //create the peer info
        esp_now_peer_info_t peerInfo;
        peerInfo.channel = 1;
        peerInfo.encrypt = false;
        if (sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &peerInfo.peer_addr[0], &peerInfo.peer_addr[1], &peerInfo.peer_addr[2], &peerInfo.peer_addr[3], &peerInfo.peer_addr[4], &peerInfo.peer_addr[5]) != 6) {
            #ifdef isDebug
            Serial.println("Failed to parse mac address");
            #endif
        }else{
            //add the peer
            if (esp_now_add_peer(&peerInfo) != ESP_OK) {
                #ifdef isDebug
                Serial.println("Failed to add peer");
                #endif
            }else{
                #ifdef isDebug
                Serial.println("Peer added");
                #endif
            }
        } */
        response->print("\r\n1"); //for now we just send back a id of 1
    }

    //send the response
    request->send(response);
}
