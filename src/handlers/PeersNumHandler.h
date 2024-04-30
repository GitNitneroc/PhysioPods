#include "ESPAsyncWebServer.h"
/*
    * This handler returns the number of connected peers (ie : the number of ClientPods connected to the ServerPod)
*/
class PeersNumHandler : public AsyncWebHandler {

private:

        
public:
    PeersNumHandler();
    virtual ~PeersNumHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
};
