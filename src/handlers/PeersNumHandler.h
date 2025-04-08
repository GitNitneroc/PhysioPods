#include "ESPAsyncWebServer.h"
/*
    * This handler returns the number of connected peers (ie : the number of ClientPods connected to the ServerPod)
*/
class PeersNumHandler : public AsyncWebHandler {

private:

        
public:
    PeersNumHandler();
    virtual ~PeersNumHandler() {}

    bool canHandle(AsyncWebServerRequest *request) const override;

    void handleRequest(AsyncWebServerRequest *request);
};
