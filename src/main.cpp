#include "ServerPod.h"
#include "ClientPod.h"

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

PhysioPod* pod = nullptr;
bool shouldBeClient = false;

//TODO : mode leurres dans fastPress
//TODO : lightmode rainbow
//TODO : physioPodMode aller/retour...
//TODO : date et heure de compilation dans le mac address


//TODO : on peut obtenir l'adresse mac du serveur simplement avec WiFi.BSSID()

void createPod();

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    Serial.print("Booting, version ");
    Serial.println(VERSION);
    Serial.println("compiled " __DATE__ " " __TIME__ );
    Serial.print("Executing on core ");
    Serial.println(xPortGetCoreID());
    #endif

    PhysioPod::initLEDs(); //initialize the LEDs, this cannot be done in createPod because we don't know if we are a server or a client yet
    //some color correction can be done here if needed on your setup (include FastLED.h to use this) :
    //FastLED.setCorrection(TypicalLEDStrip);
    //FastLED.setTemperature( Tungsten100W );
    //FastLED.setBrightness(200); //uint8_t value for scale

    //WiFi.persistent(false); //we have to use the default persistent mode, because we use esp_wifi.h functions

    shouldBeClient = PhysioPod::searchOtherPhysioWiFi();//this is a blocking call
    if (shouldBeClient){
        Serial.println("Server found ! Starting as a client...");
    }else{
        Serial.println("No server found ! Starting as a server...");
    }

    Serial.println("Creating the pod...");
    createPod(); //pod should not be nullptr anymore
}

void createPod(){
    if (shouldBeClient){
        pod = new ClientPod();
    }else{
        ScoreStorage::init();
        ServerPod* serverPod = new ServerPod();
        pod = serverPod;

        Serial.println("Adding the web handlers to the server...");
        //TODO : this could be done in the ServerPod constructor
        //handlers for the web server
        serverPod->server.addHandler(new ModeInfoHandler()); //Handles the requests for informations about the current mode
        serverPod->server.addHandler(new PeersNumHandler()); //Handles the requests for the number of connected peers
        serverPod->server.addHandler(new ModeStopHandler()); //Handles the mode stop request
        serverPod->server.addHandler(new ModeLaunchHandler()); //Handles the mode launch request
        serverPod->server.addHandler(new ServerRegistrationHandler()); //Handles the server mac address request
        serverPod->server.addHandler(new LEDRequestHandler()); //Handles the LED control requests
        serverPod->server.addHandler(new ScoreJSONHandler()); //Handles the score requests
        serverPod->server.addHandler(new SSIDHandler()); //Handles the ssid change request
        //serverPod->server.addHandler(new CaptiveRequestHandler());//call last, if no specific handler matched. Serves captivePortal.html 
    }
    //make the pod blink once, to show that it is ready, and to be sure that LED(s) are working and off when we start
    pod->setOwnLightState(true, (shouldBeClient? CRGB::Blue : CRGB::Green) , LightMode::PULSE_ON_OFF_LONG); //Note : Pulse reverts to off state after the pulse
}

void loop(){
    //we could add a small delay, especially if we are a client
    pod->updatePod();
}