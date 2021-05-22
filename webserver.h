#pragma once
#include <functional>
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
    constexpr static FrontEnd api = FrontEnd::HTTP_API;

    const String serial;
    const String model;
    const String manufacturer;

  public:
    WebServer(unsigned port, const String _manufacturer, const String _model, const String _serial);
    ~WebServer();
    void begin();
    void setFontURL(const String url);                                  // This is not input checked!
    void setNotFound(ArRequestHandlerFunction fn);                      // Custom Not Found Handle
    void enablePromMetrics(const bool enable = true);                   // Enable Prometheus Page

  private:
    AsyncWebHandler& setPromPage();                                     // Prometheus Web Contents
    AsyncWebHandler& setRootPage();                                     // Domain Root
    AsyncWebHandler& setWebPageRaw(String uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);
    AsyncWebHandler& setWebPage(String uri, const String content_type, std::function<String()> content_fn);
    void sendResponse(AsyncWebServerRequest *request, const String content, const String content_type) const;
    void createDeviceSpecificWebpages(SmartDevice* device);
};
