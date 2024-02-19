#include "isDebug.h"
#include "StaticHtmlHandler.h"
#include "ESPAsyncWebServer.h"

/*
    * This is a request handler to get the different static html pages available.
    * to add a page, add the url to the urls array and the corresponding html to the htmls array
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
            #ifdef isDebug
            Serial.println("StaticHtmlHandler received a request for : " + urls[i]);
            #endif
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