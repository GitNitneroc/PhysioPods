#include "ESPAsyncWebServer.h"
/*
    * This is a request handler for the static html files.
    * It is used to serve the html files that are not expected to change.
    * it could be extended to serve other types of files, but for now it will only serve html files.
*/
class StaticHtmlHandler : public AsyncWebHandler {
public:
    StaticHtmlHandler();
    virtual ~StaticHtmlHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);

private:
    static const int SIZE = 2;
    String urls[SIZE];
    String* htmls[SIZE];
};
