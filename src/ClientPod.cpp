#include "ClientPod.h"

//The libraries we need
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include <esp_now.h>

#include "Messages.h"
using namespace Messages;

#include <Preferences.h>

uint8_t ClientPod::serverTimer = 0;

//This will initialize a new Client Pod. The wifi can already be connected to the PhysioPod network, or not, it will connect if needed
ClientPod::ClientPod() {
    instance = this; //initialize the instance, so that the static method can access non-static members

    Serial.println("Starting as a client");
    //check if we are already connected to the PhysioPod network
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid){
        #ifdef isDebug
        Serial.println("|-Already connected to the PhysioPod network !");
        #endif
    }else{
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        #ifdef isDebug
        Serial.print("|-Connecting to WiFi as a client");
        #endif
        if (PhysioPod::searchOtherPhysioWiFi()){
            #ifdef isDebug
            Serial.println("  |-Connected to WiFi");
            #endif
        }else{
            #ifdef isDebug
            Serial.println("  |-Failed to connect to WiFi, restarting the device");
            #endif
            ESP.restart();
        }
    }

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

    //start the ping task
    serverTimer = 0;
    xTaskCreate(PingServer,"PingTask", 2000, NULL, 1, NULL); //TODO measure more precisely how much memory is needed
    Serial.println("  |-Ping Task created");

    //initialize the control
    PhysioPod::CreateControl();
    control->initialize(onControlPressed);
    Serial.println("|-Control initialized");

    Serial.println("ClientPod seems ready !");
}

/*
* This sends pings to the server in a while loop
*/
void ClientPod::PingServer(void * pvParameters){
    //this should be run in its own task
    //create the ping
    PingMessage pingMsg;
    pingMsg.sessionId = instance->getSessionId();
    while (true){
        vTaskDelay(PING_INTERVAL / portTICK_PERIOD_MS);
        pingMsg.id = static_cast<ClientPod*>(getInstance())->id;//the id is susceptible to change, so we need to get it every time
        //send the ping, there is no callback
        //esp_err_t result = esp_now_send(((ClientPod*)instance)->serverMac, (uint8_t *) &pingMsg, sizeof(PingMessage));
        esp_err_t result = esp_now_send(static_cast<ClientPod*>(instance)->serverMac, reinterpret_cast<uint8_t*>(&pingMsg), sizeof(PingMessage));
        if (result == ESP_OK) {
            #ifdef isDebug
            //Serial.println("Sent the ping message");
            #endif
        } else {
            #ifdef isDebug
            Serial.print("Error sending the ping message: ");
            Serial.println(esp_err_to_name(result));
            #endif
        }

        //check if the server responded
        serverTimer++;
        if (serverTimer >=2){
            uint8_t restartDelay = 2; //add a 2 sec delay to let the pod n1 restart first
            if (getInstance()->getId() == 1){
                restartDelay = 0;
            }
            #ifdef isDebug
            Serial.print("Server didn't respond to the ping, restarting the device in ");
            Serial.print(restartDelay);
            Serial.println(" seconds");
            #endif
            delay(restartDelay*1000+PING_INTERVAL);//This delay is to avoid all the pods restarting at the same time
            //PING_INTERVAL is added to make sure the client with id 1 has detected the timeout too
            esp_restart();
        }
    }
}

/* This method is called when the pod is in an error state
*  It should be used when restarting is unlikely to solve the problem
*  It will blink the light in red
*/
void ClientPod::displayError(){
    bool state = false;
    while(true){
        PhysioPod::setOwnLightState(state, CRGB::Red);
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
            Serial.println("-Color : "+String(ledMessage->r)+","+String(ledMessage->g)+","+String(ledMessage->b));
            Serial.println("-Mode : "+String(ledMessage->mode));
            #endif
            #ifdef USE_NEOPIXEL
            PhysioPod::setOwnLightState(ledMessage->state, CRGB(ledMessage->r, ledMessage->g, ledMessage->b), static_cast<LightMode>(ledMessage->mode));
            #else
            PhysioPod::setOwnLightState(ledMessage->state);
            #endif
            break;
        }
        case ID_REORG:{
            IdReorgMessage* reorgMessage = (IdReorgMessage*)message.messageData;
            #ifdef isDebug
            Serial.println("Received an ID reorganization message");
            Serial.println("-Old id : "+String(reorgMessage->oldId));
            Serial.println("-New id : "+String(reorgMessage->newId));
            #endif
            if (reorgMessage->oldId == ((ClientPod*)instance)->id){
                ((ClientPod*)instance)->id = reorgMessage->newId;
                #ifdef isDebug
                Serial.println("This pod's id has changed");
                #endif
            }
            break;
        }
        case PING:{
            //the getMessageType() used above already checked if the message was for this pod,
            //it would be ERROR or NOT_FOR_ME otherwise
            serverTimer = 0;//reset the server timer
            break;
        }
        case SSID:{
            SSIDMessage* ssidMessage = (SSIDMessage*)message.messageData;
            #ifdef isDebug
            Serial.println("Received an SSID message");
            Serial.println("-SSID : "+String(ssidMessage->ssid));
            #endif
            //update the ssid in the preferences
            Preferences preferences;
            preferences.begin("PhysioPod", false);
            preferences.putUInt("ssid", ssidMessage->ssid);
            preferences.end();
            //we could restart here, but let's just wait for the ping timeout
            break;
        }
        default:
            break;
    }
}

void ClientPod::updatePod(){
    bool state = control->checkControl(); //storing the state might have no use...
}
