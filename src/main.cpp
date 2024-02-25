#include "isDebug.h" //this file is used to define the isDebug variable
#include "pins.h" //this file is used to define the pins

#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"

//Our handlers for the web server
#include "handlers/LEDRequestHandler.h"
#include "handlers/CaptiveRequestHandler.h"
#include "handlers/ServerMacAddressHandler.h"
#include "handlers/StaticHtmlHandler.h"
#include "handlers/ModeLaunchHandler.h"
#include "handlers/CSSRequestHandler.h"
#include "handlers/ScoreJSONHandler.h"

//Our control
#include "controls/ButtonControl.h"

//Our modes
#include "modes/FastPressMode.h"

#include "scoreStorage.h"

//WIFI settings :
const char* ssid = "PhysioPods";
const char* password = "0123456789";

DNSServer* dnsServer = nullptr;
AsyncWebServer server(80);
String html = String(
#include "./html/index.html"
);

ButtonControl* control = new ButtonControl(BUTTON_PIN);
ScoreStorage scoreStorage = ScoreStorage();
ScoreStorage* PhysioPodMode::scoreStorage = nullptr;

PhysioPodMode* mode = nullptr;

//The id of the pod, if it's a client.
uint8_t podId = 0;

/* This can be called to start the specified PhysioPodMode*/
void startMode(PhysioPodMode* newMode){
    //if there is a mode running, stop it
    if (mode != nullptr){
        #ifdef isDebug
        Serial.println("Stopping older mode...");
        #endif
        mode->stop();
        delete mode;
    }
    #ifdef isDebug
    Serial.println("Free memory : "+String(ESP.getFreeHeap())+" bytes");
    Serial.println("Starting mode...");
    #endif
    //switch to the new mode, and start it
    mode = newMode;
    mode->start();
}

/*
    * This function is called when the device is not able to connect to the WiFi as a client
    * It will start the device as a server, and initialize the web server
*/
void startAsServer(){
    #ifdef isDebug
    Serial.println("Starting as a server");
    #endif
    //stop the WiFi client
    WiFi.disconnect();
    delay(1000); //not sure if this is necessary

    //initialize the WiFi hotspot
    Serial.println("Hotsport starting...");
    WiFi.mode(WIFI_AP);
    if(!WiFi.softAP(ssid,password,1,0,255)){//SSID, password, channel, hidden, max connection
        //if the hotspot failed to start, restart the device
        #ifdef isDebug
        Serial.println("Failed to start the hotspot, restarting the device");
        #endif
        ESP.restart();
    }

    //set the IP address of the hotspot
    delay(100); //a small delay is necessary, or check for SYSTEM_EVENT_AP_START
    Serial.println("Set softAPConfig");
    IPAddress Ip(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);
    #ifdef isDebug
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    #endif

    //start the DNS server
    Serial.println("DNS server starting...");
    dnsServer = new DNSServer();
    dnsServer->start(53, "*", WiFi.softAPIP());

    //initialize the score storage
    PhysioPodMode::scoreStorage = &scoreStorage;

    //handlers for the web server
    //TODO : create a handler where other physioPods can send their mac addresses and register themselves to the server, and get the server mac address, this will be used for ESPNow
    server.addHandler(new StaticHtmlHandler()); //Handles the static html pages requests
    server.addHandler(new CSSRequestHandler()); //Handles the CSS requests
    server.addHandler(new ModeLaunchHandler(startMode, control)); //Handles the mode launch request
    server.addHandler(new ServerMacAddressHandler()); //Handles the server mac address request
    server.addHandler(new LEDRequestHandler(&html)); //Handles the LED control requests
    server.addHandler(new ScoreJSONHandler(&scoreStorage)); //Handles the score requests
    server.addHandler(new CaptiveRequestHandler(&html));//call last, if no specific handler matched

    Serial.println("Web server starting...");
    server.begin();
}

/*
    * This function is called when the device is able to connect to the WiFi as a client
*/
void startAsClient(){
    //TODO : send a request to the server to get the server mac address
    //maybe we should send the server our own mac address, it's not available from ESPAsyncTCP library
    #ifdef isDebug
    Serial.println("Connecting to WiFi as a client");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        #ifdef isDebug
        Serial.print(".");
        #endif
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
            Serial.println(line);
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

    //id now
    line = client.readStringUntil('\n');
    if (line == ""){
        #ifdef isDebug
        Serial.println("No id provided, restarting the device");
        #endif
        ESP.restart();
    }
    podId = line.toInt();
    #ifdef isDebug
    Serial.println("Pod id : "+String(podId));
    #endif

    //disconnect from the http
    client.stop();

}

/*
    * This function is called to search for other PhysioPods
    * It will scan for WiFi networks and look for the PhysioPod network
    * It will return true if it found a PhysioPod network, and false otherwise
*/
bool searchOtherPhysioWiFi(){
    #ifdef isDebug
    Serial.println("Scanning for other PhysioPods...");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    bool found = false;
    for (int i = 0; i < n; i++){
        #ifdef isDebug
        Serial.println(WiFi.SSID(i));
        #endif
        if (WiFi.SSID(i) == ssid){
            #ifdef isDebug
            Serial.println("Found a PhysioPod network");
            #endif
            found = true;
            break;
        }
    }
    WiFi.scanDelete();
    WiFi.disconnect();
    delay(100);
    return found;
}

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    //delay(7000); //My esp module Serial is messed up after upload, so I need to wait for it to boot up
    Serial.println("Booting");
    #endif

    //initialize the LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);


    //Look for other physioPod wifi network
    if (!searchOtherPhysioWiFi()){
        #ifdef isDebug
        Serial.println("No PhysioPod network found");
        #endif
        startAsServer();
    }else{
        startAsClient();
    }

    //initialize the control
    control->initialize();
}

void loop(){
    if (dnsServer != nullptr){
        dnsServer->processNextRequest();//look for an asynchronous system rather than this one ?
    }

    //update the game mode if there is one started
    if (mode != nullptr){
        mode->update();
    }

    //TODO : consider a timer to go easy on the battery ?
}