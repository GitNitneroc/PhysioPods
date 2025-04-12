#include "ESPAsyncWebServer.h"
#include "scoreStorage.h"
/*
    * This is a request handler for the captive portal.
*/
class ScoreJSONHandler : public AsyncWebHandler {
public:
    ScoreJSONHandler();
    virtual ~ScoreJSONHandler() {}

    bool canHandle(AsyncWebServerRequest *request) const override;

    void handleRequest(AsyncWebServerRequest *request);
};
