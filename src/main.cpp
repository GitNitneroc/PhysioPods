#include "isDebug.h" //this file is used to define the isDebug variable
#include "ServerPod.h"
#include "ClientPod.h"

PhysioPod* pod = nullptr;

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    //delay(7000); //My esp module Serial is messed up after upload, so I need to wait for it to boot up
    Serial.println("Booting");
    #endif

    //initialize the LED
    pinMode(LED_PIN, OUTPUT);

    if (PhysioPod::searchOtherPhysioWiFi()){
        pod = new ClientPod();
    }else{
        ScoreStorage::init();
        ServerPod* serverPod = new ServerPod();
        pod = serverPod;

        Serial.println("Adding the web handler to the server...");
        //handlers for the web server
        serverPod->server.addHandler(new StaticHtmlHandler()); //Handles the static html pages requests
        serverPod->server.addHandler(new CSSRequestHandler()); //Handles the CSS requests
        serverPod->server.addHandler(new ModeLaunchHandler(serverPod->startMode, serverPod->control)); //Handles the mode launch request
        serverPod->server.addHandler(new ServerMacAddressHandler(&serverPod->peersNum)); //Handles the server mac address request
        serverPod->server.addHandler(new LEDRequestHandler(serverPod->setPodLightState)); //Handles the LED control requests
        serverPod->server.addHandler(new ScoreJSONHandler()); //Handles the score requests
        serverPod->server.addHandler(new CaptiveRequestHandler());//call last, if no specific handler matched */
    }

    //make sure the light is off
    pod->setOwnLightState(false);
}

void loop(){
    pod->updatePod();

    //TODO : consider a timer to go easy on the battery ?
}