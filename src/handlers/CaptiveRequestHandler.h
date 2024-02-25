#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class CaptiveRequestHandler : public AsyncWebHandler {
private:
    String* html;

public:
    CaptiveRequestHandler();
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};
