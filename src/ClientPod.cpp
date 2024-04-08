#include "ClientPod.h"
#include "isDebug.h"

//The libraries we need
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <esp_now.h>

//Our control
#include "controls/ButtonControl.h"
#include "modes/PhysioPodMode.h"

#include "Messages.h"
using namespace Messages;

//The number of attempts a clientpod makes to connect to the WiFi before restarting
#define LIMIT_CONNECTION_ATTEMPTS 20 


//TODO : this is a bit of a mess, we should refactor this, each initialization step should be in a separate method
ClientPod::ClientPod() {
    instance = this; //initialize the instance, so that the static method can access non-static members

    //initialize the control
    control = new ButtonControl(BUTTON_PIN);
    control->initialize(onControlPressed);
    PhysioPodMode::setControl(control);

    Serial.println("Starting as a client");
    #ifdef isDebug
    Serial.println("|-Connecting to WiFi as a client\n|");
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        #ifdef isDebug
        Serial.print("-");
        #endif
        if (i++ > LIMIT_CONNECTION_ATTEMPTS){
            #ifdef isDebug
            Serial.println("Failed to connect to WiFi, restarting the device");
            #endif
            ESP.restart();
        }
    }
    #ifdef isDebug
    Serial.println("|-Connected to WiFi");
    #endif

    //get the server mac address
    WiFiClient client;
    if (!client.connect("192.168.1.1", 80)){
        #ifdef isDebug
        Serial.println("Failed to connect to server, restarting the device");
        #endif
        ESP.restart();
    }
    #ifdef isDebug
    Serial.println("|-Connected to server");
    #endif
    client.print("GET /serverRegistration?mac="+WiFi.macAddress()+"&version="+VERSION+" HTTP/1.1\r\nConnection: close\r\n\r\n");
    while (client.connected()){
        if (client.available()){
            //this is the response header, we don't need it
            String line = client.readStringUntil('\n');
            /* Serial.println(line); */
            if (line == "\r"){
                break;
            }
        }
    }
    //this is the response body, the server mac address and id
    String line = client.readStringUntil('\n');
    #ifdef isDebug
    Serial.println("  |-Server mac address : "+line);
    #endif
    if (WiFi.macAddress()==line){
        #ifdef isDebug
        Serial.println("This Pod has the same mac address as the server, restarting...");
        #endif
        displayError();
    }
    
    //parse the server mac address
    if (sscanf(line.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &serverMac[0], &serverMac[1], &serverMac[2], &serverMac[3], &serverMac[4], &serverMac[5]) != 6) {
        #ifdef isDebug
        Serial.println("Failed to parse server mac address, restarting the device");
        #endif
        displayError();
    }
    //read sessionId now
    line = client.readStringUntil('\n');
    if (line == ""){
        #ifdef isDebug
        Serial.println("No session id provided, restarting the device");
        #endif
        ESP.restart();
    }
    sessionId = line.toInt();
    #ifdef isDebug
    Serial.println("  |-Session id : "+String(sessionId));
    #endif

    //read clientId now
    line = client.readStringUntil('\n');
    if (line == ""){
        #ifdef isDebug
        Serial.println("No client id provided, restarting the device");
        #endif
        ESP.restart();
    }
    id = line.toInt();
    #ifdef isDebug
    Serial.println("  |-Pod id : "+String(id));
    #endif

    //disconnect from the http
    client.stop();
    WiFi.disconnect();

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        #ifdef isDebug
        Serial.println("Error initializing ESP-NOW, restarting the device");
        #endif
        ESP.restart();
    }
    uint32_t version = 0;
    esp_now_get_version(&version);
    Serial.println("|-ESP-NOW v"+String(version)+" initialized");

    //add the server mac address to the peers
    esp_now_peer_info_t peerInfo;
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    memset(&peerInfo, 0, sizeof(peerInfo)); //this seems to be necessary !
    memcpy(peerInfo.peer_addr, serverMac, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add server as peer, restarting the device");
        ESP.restart();
    }else{
        Serial.println("  |-Server added as peer");
    }

    esp_now_register_recv_cb(this->OnDataReceived);

    //initialize the control
    control = new ButtonControl(BUTTON_PIN);
    control->initialize(onControlPressed);
    Serial.println("|-Control initialized");


    Serial.println("ClientPod seems ready !");
}

/* This method is called when the pod is in an error state
*  It should be used when restarting is unlikely to solve the problem
*  It will blink the light in red
*/
void ClientPod::displayError(){
    bool state = false;
    while(true){
        PhysioPod::setOwnLightState(state, 255,0,0);
        state = !state;
        delay(75);
    }
}

void ClientPod::onControlPressed(){
    #ifdef isDebug
    Serial.println("This pods' Control is activated !");
    #endif
    //Create a message
    ControlMessage message;
    message.id = ((ClientPod*)getInstance())->id;
    message.sessionId = instance->getSessionId();
    message.state = true;
    //send the message, there is no callback for now...
    esp_err_t result = esp_now_send(((ClientPod*)instance)->serverMac, (uint8_t *) &message, sizeof(ControlMessage));
    if (result == ESP_OK) {
        #ifdef isDebug
        Serial.println("Sent the control press message");
        #endif
    } else {
        #ifdef isDebug
        Serial.print("Error sending the control press message : ");
        Serial.println(esp_err_to_name(result));
        #endif
    }
}

// callback function that will be executed when data is received
void ClientPod::OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len) {
    parsedMessage message = getMessageType(sender_addr, data, len);
    switch (message.type){
        case ERROR:
            Serial.println("Error parsing the message (maybe wrong session)");
            break;
        case NOT_FOR_ME:
            break;
        case LED:{
            LEDMessage* ledMessage = (LEDMessage*)message.messageData;
            #ifdef isDebug
            Serial.println("Received a LED message");
            Serial.println("-Target pod : "+String(ledMessage->id));
            Serial.println("-State : "+String(ledMessage->state));
            #endif
            #ifdef USE_NEOPIXEL
            PhysioPod::setOwnLightState(ledMessage->state, ledMessage->r, ledMessage->g, ledMessage->b);
            #else
            PhysioPod::setOwnLightState(ledMessage->state);
            #endif
            break;
        }
        default:
            break;
    }
}

void ClientPod::updatePod(){
    bool state = control->checkControl(); //storing the state might have no use...
}
