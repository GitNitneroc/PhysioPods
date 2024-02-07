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

#define LED 4
#define isDebug true

DNSServer dnsServer;
AsyncWebServer server(80);
String html = String(
#include "index.html"
);

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    delay(7000); //My esp module Serial is messed up after upload, so I need to wait for it to boot up
    Serial.println("Booting");
    #endif

    //initialize the LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    //initialize the WiFi hotspot and DNS server
    Serial.println("Hotsport starting...");
    WiFi.softAP("PhysioPods","0123456789",1,0,1);
    Serial.println("DNS server starting...");
    dnsServer.start(53, "*", WiFi.softAPIP());

    //handlers for the web server
    server.addHandler(new LEDRequestHandler(LED, &html));
    server.addHandler(new CaptiveRequestHandler(&html));//call last, if no specific handler matched

    Serial.println("Web server starting...");
    server.begin();
}

void loop(){
    dnsServer.processNextRequest();
}