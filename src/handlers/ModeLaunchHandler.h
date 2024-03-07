#ifndef MODELAUNCHHANDLER_H
#define MODELAUNCHHANDLER_H

#include "ESPAsyncWebServer.h"
#include "modes/PhysioPodMode.h"
#include "controls/PhysioPodControl.h"

/*
    * This is a request handler for the captive portal.
*/
class ModeLaunchHandler : public AsyncWebHandler {
private:
    String* htmlSuccess;
    String* htmlFail;
    PhysioPodControl *control;

    void sendResponse(AsyncWebServerRequest *request, String* html);
    void (*startMode)(PhysioPodMode* mode);
    void launchNewMode(PhysioPodMode* mode);

public:
    ModeLaunchHandler(void (*startMode)(PhysioPodMode* mode), PhysioPodControl *control);
    virtual ~ModeLaunchHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

};

#endif