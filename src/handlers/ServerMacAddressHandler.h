#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class ServerMacAddressHandler : public AsyncWebHandler {

private:
    uint8_t * peersNum;
        
public:
    ServerMacAddressHandler(uint8_t * peersNum);
    virtual ~ServerMacAddressHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
    
};
