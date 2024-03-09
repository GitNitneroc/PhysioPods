#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class ModeInfoHandler : public AsyncWebHandler {

private:

        
public:
    ModeInfoHandler();
    virtual ~ModeInfoHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};
