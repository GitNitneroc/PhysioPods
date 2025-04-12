#include "ServerRegistrationHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <esp_now.h>
#include "ServerPod.h"
#include "debugPrint.h"

/*
    * This is a request handler to get the server's mac address
*/
ServerRegistrationHandler::ServerRegistrationHandler() {
}

bool ServerRegistrationHandler::canHandle(AsyncWebServerRequest *request) const{
    if (request->url()=="/serverRegistration") { //Remember this is hard coded in the client connection of pods too
        #ifdef isDebug
        DebugPrintln("ServerRegistrationHandler request !");
        #endif
        return true;
    }
    return false;
}

void ServerRegistrationHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    
    //did the client provide a version ?
    const AsyncWebParameter* clientVersion = request->getParam("version");
    if (clientVersion != NULL) {
        #ifdef isDebug
        DebugPrint("ClientPod version : ");
        DebugPrintln(clientVersion->value());
        #endif
        //if version is older than the server's, the client should not connect
        if (clientVersion->value().toInt() != VERSION) {
            DebugPrintln("Incompatible version, client should not connect");
            response->print("Incompatible version\r\n");
            request->send(response);
            return;
        }
    }else{
        #ifdef isDebug
        DebugPrintln("No version provided");
        #endif
    }
    
    //print the server's mac address and the session id
    response->print(WiFi.macAddress());
    response->print("\r\n");
    response->print(ServerPod::getInstance()->getSessionId());
    response->print("\r\n");

    //add an extra line to the response if the client provided a mac address
    const AsyncWebParameter* clientMac = request->getParam("mac");
    if (clientMac == NULL) {
        #ifdef isDebug
        DebugPrintln("This is not a pod : No mac address provided");
        #endif
    }else{
        //a new peer is connected !
        ServerPod::getInstance()->clientsTimers[ServerPod::peersNum]=0;
        (ServerPod::peersNum)++;

        #ifdef isDebug
        //read the mac address
        const char* macStr = clientMac->value().c_str();
        DebugPrint("ClientPod mac address : ");
        DebugPrint(macStr);
        DebugPrintf(" is attributed id : %d\n", ServerPod::peersNum);
        #endif
        
        //append the id to the response on a new line
        response->print(ServerPod::peersNum);
        response->print("\r\n");
    }

    //send the response
    request->send(response);
}
