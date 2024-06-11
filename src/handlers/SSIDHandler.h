#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class SSIDHandler : public AsyncWebHandler {

private:

        
public:
    SSIDHandler();
    virtual ~SSIDHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};
