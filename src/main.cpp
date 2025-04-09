#include "ServerPod.h"
#include "ClientPod.h"
#include "debugPrint.h"


PhysioPod* pod = nullptr;
bool shouldBeClient = false;

//TODO : physioPodMode aller/retour...
//TODO : date et heure de compilation dans le mac address
//TODO : préciser dans ModeChoice s'il y a un mode en cours que ça va l'arrêter
//TODO : dans le mode ChasePod, un timer,qui dit en combien de temps on fait un cycle
//TODO : le HIITimer
//TODO : dans fastPress permettre d'éviter la répétition du même pod
//TODO : on peut obtenir l'adresse mac du serveur simplement avec WiFi.BSSID()
//TODO : afficher des stats plus sexy dans les résultats

void createPod();

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    //TODO : remove this, it's just a stupid delay to facilitate debugging of the first steps
    //delay for the serial monitor to start
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    #endif
    
    #ifdef isDebug
    DebugPrint("Booting, version ");
    DebugPrintln(VERSION);
    DebugPrintln("compiled " __DATE__ " " __TIME__ );
    DebugPrint("Executing on core ");
    DebugPrintln(xPortGetCoreID());
    #endif

    PhysioPod::initLEDs(); //initialize the LEDs, this cannot be done in createPod because we don't know if we are a server or a client yet
    //some color correction can be done here if needed on your setup (include FastLED.h to use this) :
    //FastLED.setCorrection(TypicalLEDStrip);
    //FastLED.setTemperature( Tungsten100W );
    //FastLED.setBrightness(200); //uint8_t value for scale

    //WiFi.persistent(false); //we have to use the default persistent mode, because we use esp_wifi.h functions

    shouldBeClient = PhysioPod::searchOtherPhysioWiFi();//this is a blocking call
    if (shouldBeClient){
        DebugPrintln("Server found ! Starting as a client...");
    }else{
        DebugPrintln("No server found ! Starting as a server...");
    }

    DebugPrintln("Creating the pod...");
    createPod(); //pod should not be nullptr anymore
}

void createPod(){
    if (shouldBeClient){
        pod = new ClientPod();
    }else{
        ScoreStorage::init();
        ServerPod* serverPod = new ServerPod();
        pod = serverPod;
    }
    //make the pod blink once, to show that it is ready, and to be sure that LED(s) are working and off when we start
    pod->setOwnLightState(true, (shouldBeClient? CRGB::Blue : CRGB::Green) , LightMode::PULSE_ON_OFF_LONG); //Note : Pulse reverts to off state after the pulse
}

void loop(){
    //for battery maybe we could add a small delay, especially if we are a client ?
    pod->updatePod();
}