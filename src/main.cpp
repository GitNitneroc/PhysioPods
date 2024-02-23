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

DNSServer dnsServer;
AsyncWebServer server(80);
String html = String(
#include "./html/index.html"
);

ButtonControl* control = new ButtonControl(BUTTON_PIN);
ScoreStorage scoreStorage = ScoreStorage();
ScoreStorage* PhysioPodMode::scoreStorage = nullptr;

PhysioPodMode* mode = NULL;

/* This can be called to start the specified PhysioPodMode*/
void startMode(PhysioPodMode* newMode){
    //if there is a mode running, stop it
    if (mode != NULL){
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

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    //delay(7000); //My esp module Serial is messed up after upload, so I need to wait for it to boot up
    Serial.println("Booting");
    #endif

    //initialize the LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    //initialize the WiFi hotspot and DNS server
    Serial.println("Hotsport starting...");
    WiFi.softAP("PhysioPods","0123456789",1,0,1); //SSID, password, channel, hidden, max connection
    Serial.println("DNS server starting...");
    dnsServer.start(53, "*", WiFi.softAPIP());

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

    //initialize the control
    control->initialize();
}

void loop(){
    dnsServer.processNextRequest();//look for an asynchronous system rather than this one ?

    //update the game mode if there is one started
    if (mode != NULL){
        mode->update();
    }

    //TODO : consider a timer to go easy on the battery ?
}