#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class ServerMacAddressHandler : public AsyncWebHandler {
public:
    ServerMacAddressHandler();
    virtual ~ServerMacAddressHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

private:
    uint8_t peersNum = 0; //number of peers connected to the server
};
