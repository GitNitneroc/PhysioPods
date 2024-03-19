#include "ServerPod.h"
#include "isDebug.h"

//The libraries we need
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"
#include <esp_now.h>

//Our control
#ifdef USE_CAPACITIVE_TOUCH
    #include "controls/CapacitiveTouchControl.h"
#else
    #include "controls/ButtonControl.h"
#endif

#include "Messages.h"
using namespace Messages;

//ServerPod* ServerPod::instance = nullptr;
const uint8_t ServerPod::ip_addr_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t ServerPod::peersNum = 0;

ServerPod::ServerPod() : server(80) {
    dnsServer = nullptr;
    //generate a random session id
    sessionId = random(0, 65536);

    //initialize the control
    #ifdef USE_CAPACITIVE_TOUCH
        control = new CapacitiveTouchControl(BUTTON_PIN);
    #else
        control = new ButtonControl(BUTTON_PIN);
    #endif
    control->initialize(onControlPressed);

    PhysioPodMode* mode = nullptr;
    instance = this;

    Serial.println("Starting as a server");

    //stop the WiFi client
    WiFi.disconnect();
    delay(1000); //not sure if this is necessary

    //initialize the WiFi hotspot
    Serial.println("|-Hotsport starting...");
    WiFi.mode(WIFI_AP_STA);
    if(!WiFi.softAP(ssid,password,1,0,255)){//SSID, password, channel, hidden, max connection
        //if the hotspot failed to start, restart the device
        Serial.println("Failed to start the hotspot, restarting the device");
        ESP.restart();
    }

    //set the IP address of the hotspot
    delay(100); //a small delay is necessary, or check for SYSTEM_EVENT_AP_START (probablement WiFi.onEvent(callbackFunc, WiFiEvent_t::SYSTEM_EVENT_AP_START))
    Serial.println("|-Set softAPConfig");
    IPAddress Ip(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);
    #ifdef isDebug
    Serial.print("  |-AP IP address: ");
    Serial.println(WiFi.softAPIP());
    #endif

    //start the DNS server
    Serial.println("|-DNS server starting...");
    dnsServer = new DNSServer();
    dnsServer->start(53, "*", WiFi.softAPIP());

    Serial.println("|-Web server starting...");
    server.begin();

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW, restarting the device");
        ESP.restart();
    }
    uint32_t version = 0;
    esp_now_get_version(&version);
    Serial.println("|-ESP-NOW v"+String(version)+" initialized");

    //add broadcast mac address to the peers
    //TODO : actually check this is needed, I read somewhere that it's not necessary
    uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    // Register peer
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo)); //this seems to be necessary !
    memcpy(peerInfo.peer_addr, broadcastMac, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add broadcast as peer, restarting the device");
        ESP.restart();
    }else{
        Serial.println("Broadcast peer added");
    }

    Serial.println("ServerPod seems ready !");
}

/* This should be called to start the specified PhysioPodMode*/
void ServerPod::startMode(PhysioPodMode* newMode){
    #ifdef isDebug
    Serial.println("Free memory : "+String(ESP.getFreeHeap())+" bytes");
    Serial.println("Starting new mode...");
    #endif
    //if there is a mode running, stop it
    if (PhysioPodMode::currentMode != nullptr){
        if (PhysioPodMode::currentMode->isRunning()){
            #ifdef isDebug
            Serial.println("Stopping older mode...");
            #endif
            PhysioPodMode::currentMode->stop();
        }
        //store a pointer to the current mode, and delete it later
        PhysioPodMode *oldMode = PhysioPodMode::currentMode;
        PhysioPodMode::currentMode = newMode;
        PhysioPodMode::currentMode->start();
        #ifdef isDebug
        Serial.println("Deleting older mode...");
        #endif
        delete oldMode;
        //This seems stupid but it looks like it's necessary : this can be called from wifi interraction, and the loop function running on the other core could trigger an update before the new mode is started
    }else{
        //switch to the new mode, and start it
        PhysioPodMode::currentMode = newMode;
        PhysioPodMode::currentMode->start();
    }
}

/*Turn a pod light on or off. Use Id 0 for the server and 255 for every pod*/
void ServerPod::setPodLightState(uint8_t podId, bool ledState){
    //should the message be sent to another pod ?
    if (podId > 0) {
        //create the LED message
        LEDMessage message;        
        message.id = podId;
        message.sessionId = getInstance()->getSessionId();
        message.state = ledState;
        message.mode = 0;

        //send the message
        esp_err_t result = esp_now_send(ip_addr_broadcast, (uint8_t *) &message, sizeof(LEDMessage));
        if (result == ESP_OK) {
            #ifdef isDebug
            Serial.println("LEDMessage broadcasted (pod "+String(podId)+")");
            #endif
        } else {
            #ifdef isDebug
            Serial.print("Error sending the ESP-NOW message : ");
            Serial.println(esp_err_to_name(result));
            #endif
        }
        //check if the serverPod is one of the targets
        if (podId == 255) {
            #ifdef isDebug
            Serial.println("The ServerPod is one of the targets");
            #endif
            ServerPod::setOwnLightState(ledState);
        }
    } else {
        //the serverPod is the only target
        #ifdef isDebug
        Serial.println("The serverPod is the target");
        #endif
        ServerPod::setOwnLightState(ledState);
    }
}

void ServerPod::onControlPressed(){
    #ifdef isDebug
    Serial.println("This pods' Control is activated !");
    #endif
    //this should be transmitted to the mode, just like clientPods controls
    if (PhysioPodMode::currentMode != nullptr){
        PhysioPodMode::currentMode->onPodPressed(0);
    }
}

void ServerPod::updatePod(){
    if (dnsServer != nullptr){
        dnsServer->processNextRequest();//look for an asynchronous system rather than this one ?
    }

    //update the game mode if there is one started
    if (PhysioPodMode::currentMode != nullptr && PhysioPodMode::currentMode->isRunning()){
        PhysioPodMode::currentMode->update();
    }

    //Update the control : check if it's pressed
    //TODO : consider waiting a bit, or a small delay to go easy on the battery ?
    if (control != nullptr){
        bool state = control->checkControl();
    }else{
        #ifdef isDebug
        Serial.println("No control to update");
        #endif
    }
}
