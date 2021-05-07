#pragma once
#include "webserver.h"

WebServer::WebServer(unsigned port) {
    if (port > 65535) port = DEFAULT_PORT;
    webserver = new AsyncWebServer(port);

    // Setup root directory
    setWebPage("/", rootContent());

    // Setup Metrics
    enablePromMetrics();

    setNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
}

WebServer::~WebServer() {
    if (webserver == NULL) return;
    delete webserver;
}

void WebServer::setFontURL(const String url) {
    // TBD: URL Encoding
    font_url = url;
}

void WebServer::setNotFound(ArRequestHandlerFunction fn) {
    webserver->onNotFound(fn);
}

void WebServer::enablePromMetrics(const bool enable) {
    if (enable) {
        if (metrics_handler != NULL) return;
        String response_html = "# Prometheus Metrics";
        for (auto devices_it = smartDevices.begin(); devices_it != smartDevices.end(); ++devices_it) {
            const String name = devices_it->get_name();
            const String brightness = String(devices_it->get_brightness_percent());
            response_html += "\n" + name + " " + brightness;
        }
        metrics_handler = &setWebPage("/metrics", response_html, "text/plain");
        return;
    }

    if (metrics_handler == NULL) return;
    delete metrics_handler;
    metrics_handler = NULL;
}

AsyncWebHandler& WebServer::setWebPage(char *path, const String content, const String content_type) {
    return webserver->on(path, HTTP_GET, [this, content, content_type](AsyncWebServerRequest *request) {
        sendResponse(request, content, content_type);
    });
}

const String WebServer::rootContent() const {
    String response_html = "";
    response_html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    response_html += "<style>@font-face { font-family: rubrik; src: url(" + font_url + "); }";
    response_html += "* { font-family: rubrik; } </style></head>";
    response_html += "<body><table style='font-size: 18px;'>";
    // Loop through all Smart Devices
    for (auto devices_it = smartDevices.begin(); devices_it != smartDevices.end(); ++devices_it) {
        const String name = devices_it->get_name();
        const String brightness = String(devices_it->get_brightness_percent());
        response_html += "<tr><td style='text-align: right;'>" + name + "</td><td>" + brightness;
        response_html += "</td><td>&emsp;<a href=\"/" + name + "/";
    }
    response_html += "</table></body></html>";
    return response_html;
}

void WebServer::sendResponse(AsyncWebServerRequest* request, const String content, const String content_type) const {
    AsyncWebServerResponse *response = request->beginResponse(200, content_type, content);
    response->addHeader("System", "NodeMCU 1.0");
    response->addHeader("Product", "PC RGB Controller");
    response->addHeader("Designer", "C.H.J. WONG");
    request->send(response);
}