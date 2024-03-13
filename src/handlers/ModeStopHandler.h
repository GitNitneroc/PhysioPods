#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class ModeStopHandler : public AsyncWebHandler {

private:

        
public:
    ModeStopHandler();
    virtual ~ModeStopHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};
