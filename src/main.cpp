#include "isDebug.h" //this file is used to define the isDebug variable

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
#include "handlers/ModeChoiceHandler.h"

//Our control
#include "controls/ButtonControl.h"

//Our modes
#include "modes/FastPressMode.h"

#define LED_PIN 4
#define BUTTON_PIN 15

DNSServer dnsServer;
AsyncWebServer server(80);
String html = String(
#include "./html/index.html"
);

ButtonControl* control = new ButtonControl(BUTTON_PIN);

FastPressMode* mode;

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
    WiFi.softAP("PhysioPods","0123456789",1,0,1);
    Serial.println("DNS server starting...");
    dnsServer.start(53, "*", WiFi.softAPIP());

    //handlers for the web server
    //TODO : create a handler to start/stop the mode
    //TODO : create a handler to get the score
    //TODO : create a handler where other physioPods can send their mac addresses and register themselves to the server, and get the server mac address, this will be used for ESPNow
    server.addHandler(new ModeChoiceHandler()); //Handles the mode choice request
    server.addHandler(new ServerMacAddressHandler()); //Handles the server mac address request
    server.addHandler(new LEDRequestHandler(LED_PIN, &html)); //Handles the LED control requests
    server.addHandler(new CaptiveRequestHandler(&html));//call last, if no specific handler matched
    

    Serial.println("Web server starting...");
    server.begin();

    //initialize the control
    control->initialize();

    //start the fast press mode
    mode = new FastPressMode(control);
    mode->initialize(1000, 3000);
}

void loop(){
    dnsServer.processNextRequest();
    mode->update();
    //TODO : consider a timer to go easy on the battery ?
}