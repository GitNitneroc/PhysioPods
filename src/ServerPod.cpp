#include "ServerPod.h"

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
#include <esp_wifi.h>

#include "SPIFFS.h"
//OTA
//#include <ElegantOTA.h>
#include <PrettyOTA.h>

#include "debugPrint.h"

//Our handlers for the web server
#include "handlers/LEDRequestHandler.h"
#include "handlers/CaptiveRequestHandler.h"
#include "handlers/ServerRegistrationHandler.h"
#include "handlers/ModeLaunchHandler.h"
#include "handlers/ScoreJSONHandler.h"
#include "handlers/ModeInfoHandler.h"
#include "handlers/ModeStopHandler.h"
#include "handlers/PeersNumHandler.h"
#include "handlers/SSIDHandler.h"

#include "modes/PhysioPodMode.h"

#include "Messages.h"
using namespace Messages;

#define MAX_CLIENTS 4 //The maximum number of clients that can connect to the server
#define LOCAL_IP "http://192.168.1.1"
#define LOCAL_URL "/static/index.html"
#define LOCAL_OTA_URL "/update" //The URL for the OTA update
#define LOCAL_IP_URL LOCAL_IP LOCAL_URL 
#define LOCAL_OTA_IP_URL LOCAL_IP LOCAL_OTA_URL

#define LIMIT_AP_START_ATTEMPTS 100 //The number of attempts a serverpod makes to start the AP before restarting

//ServerPod* ServerPod::instance = nullptr;
const uint8_t ServerPod::ip_addr_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t ServerPod::peersNum = 0;

PrettyOTA OTAUpdates;

void ServerPod::OnAPStart(WiFiEvent_t event, WiFiEventInfo_t info){
    //This is used to make sure the AP is started before we continue
    ServerPod::getInstance()->APStarted = true;
}

bool ServerPod::initializeSPIFFS(){
    //start the SPIFFS
    if(!SPIFFS.begin(true)){
        DebugPrintln("An Error has occurred while mounting SPIFFS filesystem");
        return false;        
    }
    //test a file
    if (!SPIFFS.exists("/www/index.html")) {
        DebugPrintln("index.html was not found, you probably forgot to upload the files to SPIFFS");
        return false;
    }
    staticFilesEnabled = true; //set the static files to be served    
    #ifdef isDebug
    DebugPrintln("|-SPIFFS mounted successfully");
    #endif
    return true;
}

void ServerPod::prepareCaptivePortal(AsyncWebServer* server, bool sendToOTA){
    //captive portal for the server from https://github.com/CDFER/Captive-Portal-ESP32/tree/main
    // Required
    server->on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
    server->on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

    const char* redirection = sendToOTA ? LOCAL_OTA_IP_URL : LOCAL_IP_URL;

    // Background responses: Probably not all are Required, but some are. Others might speed things up?
    // A Tier (commonly used by modern systems)
    server->on("/generate_204", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });		   // android captive portal redirect
    server->on("/redirect", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });			   // microsoft redirect
    server->on("/hotspot-detect.html", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });  // apple call home
    server->on("/library/test/success.html", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });		   // legacy apple call home
    server->on("/canonical.html", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });	   // firefox captive portal call home
    server->on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
    server->on("/ncsi.txt", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });			   // windows call home

    server->on("/generate204", [redirection](AsyncWebServerRequest *request) { request->redirect(redirection); });

    // B Tier (uncommon)
    //  server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request){request->send(200);}); //chrome captive portal call home
    //  server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
    //  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
    //  server.on("/startpage",[](AsyncWebServerRequest *request){request->redirect(LOCAL_IP_URL);});
}

void ServerPod::attachHandlers(AsyncWebServer* server){
    DebugPrintln("|-Adding the web handlers to the server...");
    //handlers for the web server
    server->addHandler(new ModeInfoHandler()); //Handles the requests for informations about the current mode
    server->addHandler(new PeersNumHandler()); //Handles the requests for the number of connected peers
    server->addHandler(new ModeStopHandler()); //Handles the mode stop request
    server->addHandler(new ModeLaunchHandler()); //Handles the mode launch request
    server->addHandler(new ServerRegistrationHandler()); //Handles the server mac address request
    server->addHandler(new LEDRequestHandler()); //Handles the LED control requests
    server->addHandler(new ScoreJSONHandler()); //Handles the score requests
    server->addHandler(new SSIDHandler()); //Handles the ssid change request
}

//this is a callback when the OTA update starts
//used to unmount the SPIFFS filesystem if the OTA update is going to update the filesystem
void onOTAUpdateStart(NSPrettyOTA::UPDATE_MODE updateMode)
{
    // Is the filesystem going to be updated?
    if(updateMode == NSPrettyOTA::UPDATE_MODE::FILESYSTEM)
    {
        // Unmount SPIFFS filesystem here
        SPIFFS.end();
    }
}

ServerPod::ServerPod() : server(80) {
    dnsServer = nullptr;
    //generate a random session id
    sessionId = random(0, 65536);
    
    DebugPrintln("Starting as a server");

    //initialize the control
    PhysioPod::CreateControl();
    control->initialize(onControlPressed);
    DebugPrintln("|-Control initialized");

    PhysioPodMode* mode = nullptr;
    instance = this;

    if (!initializeSPIFFS()){
        DebugPrintln("/!\\Problem with SPIFFS, the files won't be served");
        //TODO : AFFICHER LES LEDS EN ROUGE CLIGNOTANT ?
    }

    //stop the WiFi client just to be sure
    WiFi.disconnect();
    delay(100); //This seems necessary, but I have more trouble with serial monitoring if I don't wait a bit

    #ifdef isDebug
    DebugPrint("|-SSID : ");
    #endif
    String ssid = getSSIDFromPreferences();

    //initialize the WiFi hotspot
    WiFi.mode(WIFI_AP_STA);
    //set the IP address of the hotspot
    DebugPrintln("|-Setting softAPConfig");
    IPAddress Ip(192, 168, 1, 1);
    WiFi.softAPConfig(Ip, Ip, IPAddress(255, 255, 255, 0), IPAddress(192, 168, 1, 2)); //set the IP address of the hotspot, the gateway, the subnet mask, the DHCP address could help on some phones (http://github.com/espressif/arduino-esp32/issues/4423)
    DebugPrint("|-AP IP address: ");
    DebugPrintln(WiFi.softAPIP());
    DebugPrint("|-Hotsport starting...");
    
    if(!WiFi.softAP(ssid,password,1,0,MAX_CLIENTS)){//SSID, password, channel, hidden, max connection
        //if the hotspot failed to start, restart the device
        DebugPrintln("\nFailed to start the hotspot, restarting the device");
        ESP.restart();
    }

    WiFi.onEvent(ServerPod::OnAPStart, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START); //when the AP starts, set the APStarted flag to true

    // Disable AMPDU RX on the ESP32 WiFi to fix a bug with disconnections on Android
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();

    uint8_t i = 0;
    //wait for the AP to start
    while (!APStarted ){
        i++;
        delay(100);
        DebugPrint(".");
        if (i > LIMIT_AP_START_ATTEMPTS){
            //if the hotspot failed to start, restart the device
            DebugPrintln("\nFailed to start the hotspot, restarting the device");
            ESP.restart();
        }
    }
    WiFi.removeEvent(WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START);
    DebugPrintln("\n  |-HotSpot started");

    if (staticFilesEnabled){
        prepareCaptivePortal(&server); //prepare the captive portal
    }else{
        prepareCaptivePortal(&server,true); //prepare the captive portal
    }
    

    // return 404 to webpage icon
    server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon

    // the catch all
	server.onNotFound([this](AsyncWebServerRequest *request) {
        request->redirect(this->staticFilesEnabled ? LOCAL_IP_URL : LOCAL_OTA_IP_URL); //redirect to the index.html page if local files are enabled, else redirect to OTA
		DebugPrint("server.onNotFound ");
		DebugPrint(request->host());	// This gives some insight into whatever was being requested on the serial monitor
		DebugPrint(request->url());
		DebugPrint(" this was redirected to ");
        DebugPrintln(this->staticFilesEnabled ? LOCAL_IP_URL : LOCAL_OTA_IP_URL);
	});

    //TODO : try to use mDNS instead of the DNS server
    //start the DNS server
    DebugPrintln("|-DNS server starting...");
    dnsServer = new DNSServer();
    dnsServer->setTTL(3600); //set the TTL to 1 hour
    dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);//set the error reply code to server failure default is DNSReplyCode::NonExistentDomain but ServerFailure should make clients send less retries
    dnsServer->start(53, "*", WiFi.softAPIP()); //start the DNS server on port 53, redirect all requests to the IP address of the hotspot
    xTaskCreate( DNSLoop, "DNSLoop", 2048, NULL, 1, NULL); //start the DNS loop in a separate task, no handle is required since we don't need to stop it

    // Start OTA
    //ElegantOTA.begin(&server);
    //DebugPrintln("|-ElegantOTA started");
    OTAUpdates.Begin(&server);
    PRETTY_OTA_SET_CURRENT_BUILD_TIME_AND_DATE();// Set current build time and date
    OTAUpdates.OnStart(onOTAUpdateStart);// Set callback
    DebugPrintln("|-PrettyOTA started"); 

    // Start WebSerial
    #ifdef isDebug
    DebugPrintln("|-WebSerial starting...");
    WebSerial.begin(&server);
    //webserial could also accept incomming messages
    WebSerial.onMessage([](uint8_t *data, size_t len) {
        DebugPrintf("Received %lu bytes from WebSerial: ", len);
        DebugWrite(data, len);
        DebugPrintln();
        DebugPrintln("Received Data...");
        String d = "";
        for(size_t i = 0; i < len; i++){
            d += char(data[i]);
        }
        DebugPrintln(d);
    });
    #endif

    //start the web server
    attachHandlers(&server); //attach the handlers to the server
    prepareCaptivePortal(&server); //prepare the captive portal
    DebugPrintln("|-Web server starting...");
    server.begin();
    
    if (staticFilesEnabled){
        //serve the static files from SPIFFS
        DebugPrintln("|-Serving static files...");
        server.serveStatic("/static/", SPIFFS, "/www/").setDefaultFile("/www/index.html").setCacheControl("max-age=6000"); //cache for 100 minutes
    }else{

    }

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        DebugPrintln("Error initializing ESP-NOW, restarting the device");
        ESP.restart();
    }
    uint32_t version = 0;
    esp_now_get_version(&version);
    DebugPrintln("|-ESP-NOW v"+String(version)+" initialized");

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
        DebugPrintln("Failed to add broadcast as peer, restarting the device");
        ESP.restart();
    }else{
        DebugPrintln("Broadcast peer added");
    }

    //receive callback
    esp_now_register_recv_cb(this->OnDataReceived);
    DebugPrintln("ESPNow callback registered");

    //Check the clients' timeouts
    xTaskCreate(
        CheckClientTimeouts,  /* Task function. */
        "CheckClientTimeouts",  /* String with name of task. */
        10000,  /* Stack size in bytes. */
        NULL,  /* Parameter passed as input of the task */
        1,  /* Priority of the task. */
        NULL);  /* Task handle. */
    DebugPrintln("Check clients timeouts task started");

    DebugPrintln("ServerPod seems ready !");
}

/* void ServerPod::broadcastMessage(const void* message){
    esp_err_t result = esp_now_send(ip_addr_broadcast, (uint8_t *) message, sizeof(message));
    //for debug purposes convert to SSIDMessage
    SSIDMessage* msg = (SSIDMessage*)message;
    DebugPrint("Broadcasting message of type ");
    DebugPrint(msg->type);
    DebugPrint(" with sessionId ");
    DebugPrintln(msg->sessionId);
    DebugPrint("SSID : ");
    DebugPrintln(msg->ssid);
    
    //DebugPrintln(sizeof(message));
    if (result == ESP_OK) {
        #ifdef isDebug
        DebugPrintln("Message broadcasted");
        #endif
    } else {
        #ifdef isDebug
        DebugPrint("Error sending the ESP-NOW message : ");
        DebugPrintln(esp_err_to_name(result));
        #endif
    }
} */

/*
    * This is the loop taking care of the DNS requests
*/
void ServerPod::DNSLoop(void * vpParameters){
    ServerPod* sp = ServerPod::getInstance();
    while(sp->dnsServer != nullptr){
        sp->dnsServer->processNextRequest();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void ServerPod::CheckClientTimeouts(void * vpParameters){
    while (true){
        delay(PING_INTERVAL);
        ServerPod* sp = ServerPod::getInstance();
        for (int i = 0; i < sp->peersNum; i++){
            sp->clientsTimers[i]++; 
            if (sp->clientsTimers[i] >= 2){
                DebugPrint("Timeout detected for pod : ");
                DebugPrintln(i+1);
                if (i<sp->peersNum-1){ //this is not the last pod
                    //create the reorg message
                    IdReorgMessage reorgMsg;
                    reorgMsg.newId = i+1;
                    reorgMsg.oldId = sp->peersNum;
                    reorgMsg.sessionId = sp->sessionId;
                    DebugPrint("Asking pod ");
                    DebugPrint(sp->peersNum);
                    DebugPrint(" to switch to id ");
                    DebugPrintln(i+1);
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
    DebugPrintln("Free memory : "+String(ESP.getFreeHeap())+" bytes");
    DebugPrintln("Starting new mode...");
    #endif
    //if there is a mode running, stop it
    if (PhysioPodMode::currentMode != nullptr){
        if (PhysioPodMode::currentMode->isRunning()){
            #ifdef isDebug
            DebugPrintln("Stopping older mode...");
            #endif
            PhysioPodMode::currentMode->stop();
        }
        //store a pointer to the current mode, and delete it later
        #ifdef isDebug
        DebugPrint("Deleting older mode : ");
        DebugPrintln(PhysioPodMode::currentMode->getName());
        DebugPrintln("Free memory : "+String(ESP.getFreeHeap())+" bytes");
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
        //DebugPrintln("PongMessage broadcasted (pod "+String(podId)+")");
        #endif
    } else {
        #ifdef isDebug
        DebugPrint("Error sending the ESP-NOW message : ");
        DebugPrintln(esp_err_to_name(result));
        #endif
    }
}

void ServerPod::OnDataReceived(const uint8_t * sender_addr, const uint8_t *data, int len){
    switch (len){
    case sizeof(ControlMessage):{
        ControlMessage* message = (ControlMessage*)data;
        if (message->sessionId != getInstance()->getSessionId()){
            #ifdef isDebug
            DebugPrintln("Received a control message with a wrong session id");
            #endif
            return;
        }
        #ifdef isDebug
        DebugPrintln("Received a control message from pod "+String(message->id));
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
            DebugPrintln("Received a ping with a wrong session id");
            #endif
            return;
        }
        #ifdef isDebug
        //DebugPrintln("Received a ping message from pod "+String(pingMsg->id));
        #endif
        ServerPod::getInstance()->clientsTimers[pingMsg->id-1]=0; //reset the timer for this client
        //send a pong message
        SendPong(pingMsg->id);
        break;
    }
    default:
        DebugPrint("Received a message of unknown length from ");
        for (int i = 0; i < 6; i++) {
            //DebugPrintf("%02X", sender_addr[i]);
            DebugPrintf("%02X", sender_addr[i]);
            if (i<5) DebugPrint(":");
        }
        DebugPrintln();
        break;
    }
}

/*Turn a pod light on or off. Use Id 0 for the server and 255 for every pod*/
void ServerPod::setPodLightState(uint8_t podId, bool ledState, CRGB color, LightMode mode, uint16_t modeSpecificData){
    //should the message be sent to another pod ?
    if (podId > 0) {
        //create the LED message
        LEDMessage message;        
        message.id = podId;
        message.sessionId = getInstance()->getSessionId();
        message.state = ledState;
        message.mode = mode;
        message.r = color.r;
        message.g = color.g;
        message.b = color.b;
        message.modeSpecific = modeSpecificData;

        //send the message
        esp_err_t result = esp_now_send(ip_addr_broadcast, (uint8_t *) &message, sizeof(LEDMessage));
        if (result == ESP_OK) {
            #ifdef isDebug
            DebugPrintln("LEDMessage broadcasted (pod "+String(podId)+")");
            #endif
        } else {
            #ifdef isDebug
            DebugPrint("Error sending the ESP-NOW message : ");
            DebugPrintln(esp_err_to_name(result));
            #endif
        }
        //check if the serverPod is one of the targets
        if (podId == 255) {
            #ifdef isDebug
            DebugPrintln("The ServerPod is one of the targets");
            #endif
            ServerPod::setOwnLightState(ledState, color,mode, modeSpecificData);
        }
    } else {
        //the serverPod is the only target
        #ifdef isDebug
        DebugPrintln("The serverPod is the target");
        #endif
        ServerPod::setOwnLightState(ledState, color, mode, modeSpecificData);
    }
}

void ServerPod::onControlPressed(){
    #ifdef isDebug
    DebugPrintln("This pods' Control is activated !");
    #endif
    //this should be transmitted to the mode, just like clientPods controls
    if (PhysioPodMode::currentMode != nullptr){
        PhysioPodMode::currentMode->onPodPressed(0);
    }
}

void ServerPod::updatePod(){
    //check if there is a new mode to start
    if (PhysioPodMode::modeConstructor != nullptr){
        //There is a new mode to start, this is the time to do it
        PhysioPodMode* mode = PhysioPodMode::modeConstructor();
        PhysioPodMode::modeConstructor = nullptr;
        DebugPrintln(PhysioPodMode::currentMode==nullptr?"No mode running":"Mode running");
        startMode(mode);
    }

    //update the game mode if there is one started
    if (PhysioPodMode::currentMode != nullptr && PhysioPodMode::currentMode->isRunning()){
        PhysioPodMode::currentMode->update();

        //also update the control
        //We could also wait a bit, to avoid updating the control too often
        if (control != nullptr){
            //DebugPrintln("Updating control");
            bool state = control->checkControl();
        }else{
            #ifdef isDebug
            DebugPrintln("No control to update");
            #endif
        }
    }

    #ifdef isDebug
    WebSerial.loop();
    #endif
}
