#include "isDebug.h"
#include "StaticHtmlHandler.h"
#include "ESPAsyncWebServer.h"

/*
    * This is a request handler to get the different modes available.
*/
StaticHtmlHandler::StaticHtmlHandler() : urls{"/modeChoice", "/results"}{
    htmls[0] = new String(
        #include "../html/modeChoice.html"
    );
    htmls[1] = new String(
        #include "../html/results.html"
    );
}

bool StaticHtmlHandler::canHandle(AsyncWebServerRequest *request){
    String url = request->url();
    for (int i = 0; i < SIZE; i++) {
        if (url==urls[i]) {
            return true;
        }
    }
    return false;
}

void StaticHtmlHandler::handleRequest(AsyncWebServerRequest *request) {
    String* html;
    String url = request->url();
    for (int i = 0; i < SIZE; i++) {
        if (url==urls[i]) {
            html = htmls[i];
        }
    }
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response);
}