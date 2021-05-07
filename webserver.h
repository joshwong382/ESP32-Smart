#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "smartdevice.h"

#define DEFAULT_PORT 9999

// Web Server
class WebServer {

  private:
    AsyncWebServer* webserver = NULL;
    AsyncWebHandler* metrics_handler = NULL;
    String font_url;

  public:
    WebServer(unsigned port = DEFAULT_PORT);
    ~WebServer();
    void setFontURL(const String url);                                  // This is not input checked!
    void setNotFound(ArRequestHandlerFunction fn);
    void enablePromMetrics(const bool enable = true);

  private:
    AsyncWebHandler& setWebPage(char *path, const String content, const String content_type = "text/html");
    const String rootContent() const;
    void sendResponse(AsyncWebServerRequest* request, const String content, const String content_type) const;
};
