#include "ESPAsyncWebServer.h"

/*
 * This handles the request for the LED state
*/
class LEDRequestHandler : public AsyncWebHandler {
    public:
    LEDRequestHandler(uint8_t ledPin, String* html);
    virtual ~LEDRequestHandler() {}
    uint8_t ledPin;
    String* html;

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};