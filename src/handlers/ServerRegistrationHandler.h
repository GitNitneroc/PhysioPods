#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class ServerRegistrationHandler : public AsyncWebHandler {

private:

        
public:
    ServerRegistrationHandler();
    virtual ~ServerRegistrationHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
    
};
