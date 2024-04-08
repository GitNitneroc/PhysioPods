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

    void sendResponse(AsyncWebServerRequest *request, String* html);
    void launchNewMode(PhysioPodMode* mode);

public:
    ModeLaunchHandler();
    virtual ~ModeLaunchHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

};

#endif