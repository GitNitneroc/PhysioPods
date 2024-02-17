#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class CSSRequestHandler : public AsyncWebHandler {
public:
    CSSRequestHandler();
    virtual ~CSSRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

private:
    String* css;
};
