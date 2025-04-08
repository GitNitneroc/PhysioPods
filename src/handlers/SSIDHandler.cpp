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
#include "debugPrint.h"

/*
    * This is a request handler to change the SSID of the ESP32.
*/
SSIDHandler::SSIDHandler() {
}

bool SSIDHandler::canHandle(AsyncWebServerRequest *request) const{
    if (request->url()=="/ssid") {
        #ifdef isDebug
        DebugPrintln("SSIDHandler request !");
        #endif
        return true;
    }
    return false;
}

void SSIDHandler::handleRequest(AsyncWebServerRequest *request) {
    //start the response
    AsyncResponseStream *response = request->beginResponseStream("text/plain");

    const AsyncWebParameter* ssidParam = request->getParam("ssid");
    if (ssidParam == nullptr){
        response->print("No ssid parameter found");
        DebugPrintln("No ssid parameter found");
        request->send(response);
    }else{
        //get the ssid
        int ssid = ssidParam->value().toInt();
        if (ssid < 0 || ssid > 255){
            response->print("Invalid ssid value");
            DebugPrintln("Invalid ssid value");
            request->send(response);
            return;
        }
        //change the ssid in eeprom
        #ifdef isDebug
        DebugPrint("Changing preferences ssid to ");
        DebugPrintln(ssid);
        #endif
        Preferences preferences;    
        preferences.begin("PhysioPod", false);
        preferences.putUInt("ssid", ssid);
        preferences.end();

        //send the ssid to the pods
        Messages::SSIDMessage ssidMessage ;//= {ServerPod::getInstance()->getSessionId(), (uint8_t)ssid};
        ssidMessage.sessionId = ServerPod::getInstance()->getSessionId();
        ssidMessage.ssid = (uint8_t)ssid;
        esp_now_send(ServerPod::ip_addr_broadcast, (uint8_t*)&ssidMessage, sizeof(Messages::SSIDMessage));
        #ifdef isDebug
        DebugPrintln("SSID sent to the pods");
        #endif
        
        //send the response
        response->print("Changing ssid to ");
        response->print(ssidParam->value());
        response->print("The pods will now restart to apply the changes.");
        request->send(response);

        #ifdef isDebug
        DebugPrintln("Restarting the pod...");
        #endif
        //restart the pod
        esp_restart();
    }
}
