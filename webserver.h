#ifndef webserver_h
#define webserver_h
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "rgbdevice.h"

// Web Server

void setAllWebServerPages(AsyncWebServer *server, RGBDevice *desk_led, BrightnessDevice *music_led) {

  server->onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  server->on("/", HTTP_GET, [desk_led, music_led](AsyncWebServerRequest *request) {
    String read1 = desk_led->get_power() ? String(desk_led->get_brightness_percent()) + "%" : "OFF";
    String read2 = desk_led->get_power() ? "off" : "on";
    String read3 = music_led->get_power() ? String(music_led->get_brightness_percent()) + "%" : "OFF";
    String read4 = music_led->get_power() ? "off" : "on";
    String response_html = "";
    response_html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    response_html += "<style>@font-face { font-family: rubrik; src: url(https://edev.i.lithium.com/html/assets/Rubrik_Regular.otf); }";
    response_html += "* { font-family: rubrik; } </style></head>";
    response_html += "<body><table style='font-size: 18px;'>";
    response_html += "<tr><td style='text-align: right;'>LED:</td><td>" + read1 + "</td><td>&emsp;<a href=\"/led/" + read2 + "\">Turn " + read2 + "</a></td></tr>";
    response_html += "<tr><td style='text-align: right;'>Music LED:</td><td>" + read3 + "</td><td>&emsp;<a href=\"/music/" + read4 + "\">Turn " + read4 + "</a></td></tr>";
    response_html += "</table></body></html>";
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", response_html);
    response->addHeader("System", "NodeMCU 1.0");
    response->addHeader("Product", "PC RGB Controller");
    response->addHeader("Designer", "C.H.J. WONG");
    request->send(response);
  });

  server->on("/metrics", HTTP_GET, [desk_led, music_led](AsyncWebServerRequest *request) {
    String response_html = "# Prometheus Metrics";
    response_html += "\ndesk_led " + String(desk_led->get_brightness_percent());
    response_html += "\nmusic_led " + String(music_led->get_brightness_percent());
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", response_html);
    request->send(response);
  });

  server->on("/led/on", HTTP_GET, [desk_led](AsyncWebServerRequest *request) {
    
    desk_led->set_power(PWR_ON, Device::HTTP_API);

    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      char color_arr[7];
      color.toCharArray(color_arr, 7);
      uint32_t rgb = strtol(color_arr, NULL, 16);
      desk_led->set_rgb(rgb, Device::HTTP_API);
      
      AsyncWebServerResponse *response = request->beginResponse(302);
      //response->addHeader("RGB: R: ", colorinttohexstr(r) + "G: " + colorinttohexstr(g) + "B: " + colorinttohexstr(b));
      //response->addHeader("HSV", String(hue) + "," + String(sat) + "," + String(br));
      response->addHeader("Location", "/");
      request->send(response);
      return;
    }

    int brightness = -1;
    if (request->hasParam("brightness")) {
      brightness = request->getParam("brightness")->value().toInt();
      if (brightness >= 0 || brightness <= 100) {
        desk_led->set_brightness_percent(brightness, Device::HTTP_API);
      }
    }

    request->redirect("/");
  });

  server->on("/led/off", HTTP_GET, [desk_led](AsyncWebServerRequest *request) {
    desk_led->set_power(PWR_OFF, Device::HTTP_API);
    request->redirect("/");
  });

  server->on("/led/status", HTTP_GET, [desk_led](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(desk_led->get_power()));
  });

  server->on("/led/brightness", HTTP_GET, [desk_led](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(desk_led->get_brightness_percent()));
  });

  server->on("/led/color", HTTP_GET, [desk_led](AsyncWebServerRequest *request) {
    String rgb = desk_led->get_rgb_str();
    request->send(200, "text/plain", rgb);
  });

  server->on("/music/on", HTTP_GET, [music_led](AsyncWebServerRequest *request) {
    int brightness = -1;
    if (request->hasParam("brightness")) {
      brightness = request->getParam("brightness")->value().toInt();
      if (brightness >= 0 || brightness <= 100) {
        music_led->set_brightness_percent(brightness, Device::HTTP_API);
      }
    }
    
    request->redirect("/");
  });

  server->on("/music/off", HTTP_GET, [music_led](AsyncWebServerRequest *request) {
    music_led->set_power(PWR_OFF, Device::HTTP_API);
    request->redirect("/");
  });

  server->on("/music/status", HTTP_GET, [music_led](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led->get_power()));
  });

  server->on("/music/brightness", HTTP_GET, [music_led](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led->get_brightness_percent()));
  });
}

// IP

String ip2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3]); 
}

bool isValidIP(const IPAddress& ipAddress) {
  return (ipAddress[0] > 0 && ipAddress[1] >= 0 && ipAddress[2] >= 0 && ipAddress[3] >=0 && ipAddress[0] < 255 && ipAddress[1] < 255 && ipAddress[2] < 255 && ipAddress[3] < 255);
}

/*
bool wifi_check() {

  static long attempt_connect_timer;
  if (WiFi.status() != WL_CONNECTED && millis() - attempt_connect_timer >= 10000) {
    attempt_connect_timer = millis();
    WiFi.begin(ssid, pass);
    if (debug) Serial.print("Disconnected. Reconnecting... Connection Status: ");
    if (debug) Serial.println(WiFi.status());
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  IPAddress ip = WiFi.localIP();
  if (!isValidIP(ip)) {
    if (debug) Serial.print("No IP... Connection Status: ");
    if (debug) Serial.println(WiFi.status());
    WiFi.disconnect();
    return false;
  }

  return true;
}
*/

#endif