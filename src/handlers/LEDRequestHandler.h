#include "ESPAsyncWebServer.h"

/*
 * This handles the requests to change the LED states
 * The request should be in the form of "/LED?LED=ON&id=1", the order of the parameters matters !
*/
class LEDRequestHandler : public AsyncWebHandler {
private:
    void (*setPodLightState)(uint8_t, bool);
    
public:
    LEDRequestHandler(void (*setPodLightState)(uint8_t, bool));
    virtual ~LEDRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};