#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the captive portal.
*/
class ModeChoiceHandler : public AsyncWebHandler {
public:
    ModeChoiceHandler();
    virtual ~ModeChoiceHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

private:
    String* html;
};
