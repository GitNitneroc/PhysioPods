#include "ESPAsyncWebServer.h"

/*
 * This handles the request for the LED state
*/
class LEDRequestHandler : public AsyncWebHandler {
    public:
    LEDRequestHandler();
    virtual ~LEDRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};