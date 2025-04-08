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
    /* String* htmlSuccess;
    String* htmlFail; */

    void sendSuccessResponse(AsyncWebServerRequest *request);
    void sendFailResponse(AsyncWebServerRequest *request);
    void launchNewMode(PhysioPodMode* mode);

public:
    ModeLaunchHandler();
    virtual ~ModeLaunchHandler() {}

    bool canHandle(AsyncWebServerRequest *request) const override;

    void handleRequest(AsyncWebServerRequest *request);

};

#endif