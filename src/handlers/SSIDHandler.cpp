#include "SSIDHandler.h"
#include "ESPAsyncWebServer.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <esp_now.h>
#include <Preferences.h>
#include "Messages.h"
#include "ServerPod.h"

/*
    * This is a request handler to change the SSID of the ESP32.
*/
SSIDHandler::SSIDHandler() {
}

bool SSIDHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/ssid") {
        #ifdef isDebug
        Serial.println("SSIDHandler request !");
        #endif
        return true;
    }
    return false;
}

void SSIDHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("text/plain");

    AsyncWebParameter* ssidParam = request->getParam("ssid");
    if (ssidParam == nullptr){
        response->print("No ssid parameter found");
        Serial.println("No ssid parameter found");
        request->send(response);
    }else{
        //get the ssid
        int ssid = ssidParam->value().toInt();
        //change the ssid in eeprom
        Preferences preferences;    
        preferences.begin("PhysioPod", false);
        preferences.putUInt("ssid", ssid);
        preferences.end();

        //send the ssid to the pods
        Messages::SSIDMessage ssidMessage;
        ssidMessage.sessionId = ServerPod::getInstance()->getSessionId();
        ssidMessage.ssid = ssid;
        ServerPod::broadcastMessage(&ssidMessage);
        
        //send the response
        response->print("Changing ssid to ");
        response->print(ssidParam->value());
        response->print("The pods will now restart to apply the changes.");
        request->send(response);
        //restart the pod
        esp_restart();
    }
}
