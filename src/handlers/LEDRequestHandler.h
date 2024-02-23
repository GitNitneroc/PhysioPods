#include "ESPAsyncWebServer.h"

/*
 * This handles the request for the LED state
*/
class LEDRequestHandler : public AsyncWebHandler {
    public:
    LEDRequestHandler( String* html);
    virtual ~LEDRequestHandler() {}
    String* html;

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};