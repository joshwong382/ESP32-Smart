#include "webserver.h"

WebServer::WebServer(unsigned port, const String _manufacturer, const String _model, const String _serial) :
            manufacturer {_manufacturer},
            model {_model},
            serial {_serial} {

    if (port < 1 || port > 65535) port = DEFAULT_PORT;
    webserver = new AsyncWebServer(port);

    // Setup root directory
    setRootPage();

    // Setup Metrics
    enablePromMetrics();

    setNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    font_url = "https://ca.josh-wong.net/Rubrik_Regular.otf";
}

WebServer::~WebServer() {
    if (webserver == NULL) return;
    delete webserver;
}

void WebServer::begin() {

    for (auto it = SmartDevice::alldevices.begin(); it != SmartDevice::alldevices.end(); ++it) {
        createDeviceSpecificWebpages(*it);
    }
    webserver->begin();
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
        metrics_handler = &setPromPage();
        return;
    }

    if (metrics_handler == NULL) return;
    delete metrics_handler;
    metrics_handler = NULL;
}

AsyncWebHandler& WebServer::setPromPage() {
    return setWebPage("/metrics", "text/plain", []() -> String {
        
        String html = "# Prometheus Metrics";

        // Smart Devices
        for (auto it = SmartDevice::alldevices.begin(); it != SmartDevice::alldevices.end(); ++it) {
            const String name = (*it)->getName();
            const String brightness = String((*it)->getBrightnessPercent());
            html += "\n" + name + " " + brightness;
        }

        // Smart Sensors
        for (auto it = SmartSensorBase::allsensors.begin(); it != SmartSensorBase::allsensors.end(); ++it) {
            switch ((*it)->type) {
                case SensorTypes::Weather: {
                    Weather *weather_dev = (Weather*) *it;
                    const String name = weather_dev->getName();
                    const double temp = weather_dev->getTemp();
                    const double humid = weather_dev->getHumidity();
                    if (temp > ABSOLUTE_ZERO) html += "\ntemp_" + name + " " + String(temp);
                    if (humid > 0) html += "\nhumidity_" + name + " " + String(humid);
                }
                default:
                    break;
            }
        }

        return html;
    });
}

AsyncWebHandler& WebServer::setRootPage() {
    return setWebPage("/", "text/html", [this]() -> String {
        String response_html = "";
        response_html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
        response_html += "<style>@font-face { font-family: rubrik; src: url(" + font_url + "); }";
        response_html += "* { font-family: rubrik; } </style></head>";
        response_html += "<body><table style='font-size: 18px;'>";

        // Loop through all Smart Devices
        for (auto devices_it = SmartDevice::alldevices.begin(); devices_it != SmartDevice::alldevices.end(); ++devices_it) {
            const String name = (*devices_it)->getName();
            const bool pwr = (*devices_it)->getPower();

            // check brightness (really need to clean this up)
            String on_str;
            if ((*devices_it)->type == DeviceType::SmartDevice) {
                on_str = "ON";
            } else {
                on_str = String((*devices_it)->getBrightnessPercent()) + "%";
            }
            const String brightness_str = pwr ? on_str : "OFF";
            const String flip_str = pwr ? "off" : "on";

            response_html += "<tr><td style='text-align: right;'>" + name + ":</td><td>" + brightness_str
                + "</td><td>&emsp;<a href=\"/" + name + "/" + flip_str + "\">Turn " + flip_str + "</a></td></tr>";
        }

        response_html += "</table></body></html>";
        return response_html;
    });
}

AsyncWebHandler& WebServer::setWebPageRaw(String uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest) {
    return webserver->on(uri.c_str(), method, onRequest);
}

AsyncWebHandler& WebServer::setWebPage(String uri, const String content_type, std::function<String()> content_fn) {
    return webserver->on(uri.c_str(), HTTP_GET, [this, content_fn, content_type](AsyncWebServerRequest *request) {
        String content = content_fn();
        sendResponse(request, content, content_type);
    });
}

void WebServer::sendResponse(AsyncWebServerRequest* request, const String content, const String content_type) const {
    AsyncWebServerResponse *response = request->beginResponse(200, content_type, content);
    response->addHeader("Serial", "NodeMCU 1.0");
    response->addHeader("Model", "PC RGB Controller");
    response->addHeader("Manufacturer", "C.H.J. WONG");
    request->send(response);
}

void WebServer::createDeviceSpecificWebpages(SmartDevice* device) {

    // for all DeviceType device, run a specific code
    const String base_url = "/" + device->getName() + "/";

    setWebPageRaw(base_url + "on", HTTP_GET, [device](AsyncWebServerRequest *request) {
        switch (device->type) {

            case DeviceType::RGBDevice: {
                RGBDevice *rgb_dev = (RGBDevice*) device;
                if (request->hasParam("color")) {
                    String color = request->getParam("color")->value();
                    char color_arr[7];
                    color.toCharArray(color_arr, 7);
                    uint32_t rgb = strtol(color_arr, NULL, 16);
                    rgb_dev->setRGB(rgb, FrontEnd::HTTP_API);
                }
            }

            case DeviceType::BrightnessDevice: {
                BrightnessDevice *brightness_dev = (BrightnessDevice*) device;
                int brightness = -1;
                if (request->hasParam("brightness")) {
                    brightness = request->getParam("brightness")->value().toInt();
                    if (brightness >= 0 || brightness <= 100) {
                        brightness_dev->setBrightnessPercent(brightness, FrontEnd::HTTP_API);
                    }
                }
            }

            default:
                device->setPower(PWR_ON, FrontEnd::HTTP_API);
        }

        AsyncWebServerResponse *response = request->beginResponse(302);
        response->addHeader("Location", "/");
        request->send(response);
    });

    switch (device->type) {
        case DeviceType::RGBDevice:
            setWebPage(base_url + "color", "text/plain", [device]() -> String {
                RGBDevice *rgb_dev = (RGBDevice*) device;
                return rgb_dev->getRGBStr();
            });

        case DeviceType::BrightnessDevice:
            setWebPage(base_url + "brightness", "text/plain", [device]() -> String {
                BrightnessDevice *brightness_dev = (BrightnessDevice*) device;
                return String(brightness_dev->getBrightnessPercent());
            });

        case DeviceType::SmartDevice:
            setWebPage(base_url + "status", "text/plain", [device]() -> String {
                return String(device->getPower());
            });

            setWebPageRaw(base_url + "off", HTTP_GET, [device](AsyncWebServerRequest *request) {
                device->setPower(PWR_OFF, FrontEnd::HTTP_API);
                request->redirect("/");
            });

        default:
            break;
    }
}