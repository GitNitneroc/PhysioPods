#include "ESPAsyncWebServer.h"
#include "../modes/PhysioPodMode.h"
#include "../controls/PhysioPodControl.h"

/*
    * This is a request handler for the captive portal.
*/
class ModeLaunchHandler : public AsyncWebHandler {
public:
    ModeLaunchHandler(void (*startMode)(PhysioPodMode* mode), PhysioPodControl* control);
    virtual ~ModeLaunchHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

private:
    String* htmlSuccess;
    String* htmlFail;

    void sendResponse(AsyncWebServerRequest *request, String* html);
    void (*startMode)(PhysioPodMode* mode);
    PhysioPodControl *control;
};
