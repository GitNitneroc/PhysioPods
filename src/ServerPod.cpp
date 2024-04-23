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

#include "SPIFFS.h"

//Our control
#ifdef USE_CAPACITIVE_TOUCH
    #include "controls/CapacitiveTouchControl.h"
#else
    #include "controls/ButtonControl.h"
#endif

#include "modes/PhysioPodMode.h"

#include "Messages.h"
using namespace Messages;

//ServerPod* ServerPod::instance = nullptr;
const uint8_t ServerPod::ip_addr_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t ServerPod::peersNum = 0;

ServerPod::ServerPod() : server(80) {
    dnsServer = nullptr;
    //generate a random session id
    sessionId = random(0, 65536);
    
    Serial.println("Starting as a server");

    //initialize the control
    #ifdef USE_CAPACITIVE_TOUCH
        control = new CapacitiveTouchControl(BUTTON_PIN);
    #else
        control = new ButtonControl(BUTTON_PIN);
    #endif
    control->initialize(onControlPressed);
    PhysioPodMode::setControl(control);

    PhysioPodMode* mode = nullptr;
    instance = this;

    //start the SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS, rebooting...");
        ESP.restart();        
    }
    //test a file
    if (!SPIFFS.exists("/www/index.html")) {
        Serial.println("index.html was not found, are you sure you uploaded the filesystem image ? Rebooting...");
        ESP.restart();
        return;
    }    
    #ifdef isDebug
    Serial.println("|-SPIFFS mounted successfully");
    #endif

    //stop the WiFi client just to be sure
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

    server.serveStatic("/static/", SPIFFS, "/www/").setDefaultFile("/www/index.html").setCacheControl("max-age=6000"); //cache for 100 minutes
    Serial.println("|-Static files server initialised...");

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

    //receive callback
    esp_now_register_recv_cb(this->OnDataReceived);
    Serial.println("ESPNow callback registered");

    //Check the clients' timeouts
    xTaskCreate(
        CheckClientTimeouts,  /* Task function. */
        "CheckClientTimeouts",  /* String with name of task. */
        10000,  /* Stack size in bytes. */
        NULL,  /* Parameter passed as input of the task */
        1,  /* Priority of the task. */
        NULL);  /* Task handle. */
    Serial.println("Check clients timeouts task started");

    Serial.println("ServerPod seems ready !");
}

void ServerPod::CheckClientTimeouts(void * vpParameters){
    while (true){
        delay(PING_INTERVAL);
        ServerPod* sp = ServerPod::getInstance();
        for (int i = 0; i < sp->peersNum; i++){
            sp->clientsTimers[i]++; 
            if (sp->clientsTimers[i] >= 2){
                Serial.print("Timeout detected for pod : ");
                Serial.println(i+1);
                if (i<sp->peersNum-1){ //this is not the last pod
                    //create the reorg message
                    IdReorgMessage reorgMsg;
                    reorgMsg.newId = i+1;
                    reorgMsg.oldId = sp->peersNum;
                    reorgMsg.sessionId = sp->sessionId;
                    Serial.print("Asking pod ");
                    Serial.print(sp->peersNum);
                    Serial.print(" to switch to id ");
                    Serial.println(i+1);
                    sp->clientsTimers[i] = 0; //reset the timer for the reorg'ed pod
                    //send reorg message
                    esp_now_send(ip_addr_broadcast, (uint8_t *) &reorgMsg, sizeof(IdReorgMessage));
                }
                sp->peersNum--;
            }
        }
    }
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
        #ifdef isDebug
        Serial.print("Deleting older mode : ");
        Serial.println(PhysioPodMode::currentMode->getName());
        Serial.println("Free memory : "+String(ESP.getFreeHeap())+" bytes");
        #endif
        delete PhysioPodMode::currentMode;
        PhysioPodMode::currentMode = newMode;
        PhysioPodMode::currentMode->start();
    }else{
        //switch to the new mode, and start it
        PhysioPodMode::currentMode = newMode;
        PhysioPodMode::currentMode->start();
    }
}

void ServerPod::SendPong(uint8_t podId){
    //create the pong message
    PingMessage pongMsg;
    pongMsg.id = podId;
    pongMsg.sessionId = ServerPod::getInstance()->sessionId;
    //send the message
    esp_err_t result = esp_now_send(ip_addr_broadcast, (uint8_t *) &pongMsg, sizeof(PingMessage));
    if (result == ESP_OK) {
        #ifdef isDebug
        //Serial.println("PongMessage broadcasted (pod "+String(podId)+")");
        #endif
    } else {
        #ifdef isDebug
        Serial.print("Error sending the ESP-NOW message : ");
        Serial.println(esp_err_to_name(result));
        #endif
    }
}

void ServerPod::OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len){
    switch (len){
    case sizeof(ControlMessage):{
        ControlMessage* message = (ControlMessage*)data;
        if (message->sessionId != getInstance()->getSessionId()){
            #ifdef isDebug
            Serial.println("Received a control message with a wrong session id");
            #endif
            return;
        }
        #ifdef isDebug
        Serial.println("Received a control message from pod "+String(message->id));
        #endif
        if(PhysioPodMode::currentMode != nullptr && PhysioPodMode::currentMode->isRunning()){
            //call the current mode's press callback
            PhysioPodMode::currentMode->onPodPressed(message->id);
        }
        break;
    }
    case sizeof(PingMessage):{
        PingMessage* pingMsg = (PingMessage*)data;
        if (pingMsg->sessionId != getInstance()->getSessionId()){
            #ifdef isDebug
            Serial.println("Received a ping with a wrong session id");
            #endif
            return;
        }
        #ifdef isDebug
        //Serial.println("Received a ping message from pod "+String(pingMsg->id));
        #endif
        ServerPod::getInstance()->clientsTimers[pingMsg->id-1]=0; //reset the timer for this client
        //send a pong message
        SendPong(pingMsg->id);
        break;
    }
    default:
        Serial.print("Received a message of unknown length from ");
        for (int i = 0; i < 6; i++) {
            Serial.print(sender_addr[i], HEX);
            if (i<5) Serial.print(":");
        }
        Serial.println();
        break;
    }
}

/*Turn a pod light on or off. Use Id 0 for the server and 255 for every pod*/
void ServerPod::setPodLightState(uint8_t podId, bool ledState, uint8_t r, uint8_t g, uint8_t b){
    //should the message be sent to another pod ?
    if (podId > 0) {
        //create the LED message
        LEDMessage message;        
        message.id = podId;
        message.sessionId = getInstance()->getSessionId();
        message.state = ledState;
        message.mode = 0;
        message.r = r;
        message.g = g;
        message.b = b;

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
            ServerPod::setOwnLightState(ledState, r, g, b);
        }
    } else {
        //the serverPod is the only target
        #ifdef isDebug
        Serial.println("The serverPod is the target");
        #endif
        ServerPod::setOwnLightState(ledState, r, g, b);
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
    //TODO : do this is a separate task
    if (dnsServer != nullptr){
        dnsServer->processNextRequest();
    }

    if (PhysioPodMode::modeConstructor != nullptr){
        //There is a new mode to start, this is the time to do it
        PhysioPodMode* mode = PhysioPodMode::modeConstructor();
        PhysioPodMode::modeConstructor = nullptr;
        Serial.println(PhysioPodMode::currentMode==nullptr?"No mode running":"Mode running");
        startMode(mode);
    }

    //update the game mode if there is one started
    if (PhysioPodMode::currentMode != nullptr && PhysioPodMode::currentMode->isRunning()){
        PhysioPodMode::currentMode->update();

        //also update the control
        //We could also wait a bit, to avoid updating the control too often
        if (control != nullptr){
            bool state = control->checkControl();
        }else{
            #ifdef isDebug
            Serial.println("No control to update");
            #endif
        }
    }
}
