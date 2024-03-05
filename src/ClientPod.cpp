#include "ClientPod.h"
#include "isDebug.h"
#include "pins.h"

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

#include "messages.h"

//The number of attempts a clientpod makes to connect to the WiFi before restarting
#define LIMIT_CONNECTION_ATTEMPTS 20 

ClientPod* ClientPod::instance = nullptr;

ClientPod::ClientPod() {
    instance = this; //initialize the instance, so that the static method can access non-static members

    //initialize the control
    control = new ButtonControl(BUTTON_PIN);
    control->initialize();

    #ifdef isDebug
    Serial.println("Connecting to WiFi as a client");
    #endif

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        #ifdef isDebug
        Serial.print(".");
        #endif
        if (i++ > LIMIT_CONNECTION_ATTEMPTS){
            #ifdef isDebug
            Serial.println("Failed to connect to WiFi, restarting the device");
            #endif
            ESP.restart();
        }
    }
    #ifdef isDebug
    Serial.println("Connected to WiFi");
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
    Serial.println("Connected to server");
    #endif
    client.print("GET /serverMacAddress?mac="+WiFi.macAddress()+" HTTP/1.1\r\nConnection: close\r\n\r\n");
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
    Serial.println("Server mac address : "+line);
    #endif
    if (WiFi.macAddress()==line){
        #ifdef isDebug
        Serial.println("This Pod has the same mac address as the server, restarting...");
        #endif
        //TODO : This could theoretically happend... We can't change the mac permanently
        // so we should add a new mac address to the eeprom and restart the device
        // at startup it would read the mac from eeprom and use it until next restart
    }
    
    //parse the server mac address
    if (sscanf(line.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &serverMac[0], &serverMac[1], &serverMac[2], &serverMac[3], &serverMac[4], &serverMac[5]) != 6) {
        #ifdef isDebug
        Serial.println("Failed to parse server mac address, restarting the device");
        #endif
        ESP.restart();
    }

    //read id now
    line = client.readStringUntil('\n');
    if (line == ""){
        #ifdef isDebug
        Serial.println("No id provided, restarting the device");
        #endif
        ESP.restart();
    }
    id = line.toInt();
    #ifdef isDebug
    Serial.println("Pod id : "+String(id));
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
    Serial.println("ESP-NOW v"+String(version)+" initialized");

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
        Serial.println("Server added as peer");
    }

    esp_now_register_recv_cb(this->OnDataReceived);

    Serial.println("ClientPod seems ready !");
}


// callback function that will be executed when data is received
void ClientPod::OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len) {
    LEDMessage* message = (LEDMessage*)data;
    #ifdef isDebug
    Serial.println("Received a LED message");
    Serial.println("-Target pod : "+String(message->id));
    Serial.println("-State : "+String(message->state));
    #endif
    //check if the message is for me (255 is the broadcast id, it's for everyone)
    if (message->id == instance->id || message->id == 255){
        #ifdef isDebug
        Serial.println("-This message is for me !");
        #endif
        PhysioPod::setLightState(message->state);
    }
}

void ClientPod::updatePod(){
    //TODO : check the button control
}
