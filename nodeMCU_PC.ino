#define FASTLED_INTERNAL

#define REDPIN D6
#define GREENPIN D7
#define BLUEPIN D8
#define MUSIC_TRIG D2

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include "rgbdevice.h"

// FastLED
#define DATA_PIN    5
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    24
#define FRAMES_PER_SECOND  20
CRGB leds[NUM_LEDS];
uint8_t gHue = 0; // rotating "base color" on FastLED

void setAllWebServerPages();

// WIFI
char ssid[] = "WAZZUP!!! IoT";
char pass[] = "hackintosh-grass-design-fly-clothing";
AsyncWebServer server(9999);

RGBDevice desk_led = RGBDevice("desk_led");
BrightnessDevice music_led = BrightnessDevice("music_led");

void setup() {
  //Serial.begin(115200);
  analogWriteRange(255);
  pinMode(MUSIC_TRIG, INPUT_PULLUP);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  // FastLED 
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Wi-Fi
  WiFi.mode(WIFI_STA);

  // Web Server
  setAllWebServerPages();
  server.begin();
}

void loop() {

}


// Web Server
void setAllWebServerPages() {

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String read1 = desk_led.get_power() ? String(desk_led.get_brightness_percent()) + "%" : "OFF";
    String read2 = desk_led.get_power() ? "off" : "on";
    String read3 = music_led.get_power() ? String(music_led.get_brightness_percent()) + "%" : "OFF";
    String read4 = music_led.get_power() ? "off" : "on";
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

  server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response_html = "# Prometheus Metrics";
    response_html += "\ndesk_led " + String(desk_led.get_brightness_percent());
    response_html += "\nmusic_led " + String(music_led.get_brightness_percent());
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", response_html);
    request->send(response);
  });

  server.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    desk_led.set_power(PWR_ON);

    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      char color_arr[7];
      color.toCharArray(color_arr, 7);
      uint32_t rgb = strtol(color_arr, NULL, 16);
      desk_led.set_rgb(rgb);
      
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
        desk_led.set_brightness_percent(brightness);
      }
    }

    request->redirect("/");
  });

  server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    desk_led.set_power(PWR_OFF);
    request->redirect("/");
  });

  server.on("/led/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(desk_led.get_power()));
  });

  server.on("/led/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(desk_led.get_brightness_percent()));
  });

  server.on("/led/color", HTTP_GET, [](AsyncWebServerRequest *request) {
    String rgb = desk_led.get_rgb_str();
    request->send(200, "text/plain", rgb);
  });

  server.on("/music/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    int brightness = -1;
    if (request->hasParam("brightness")) {
      brightness = request->getParam("brightness")->value().toInt();
      if (brightness >= 0 || brightness <= 100) {
        music_led.set_brightness_percent(brightness);
      }
    }
    
    request->redirect("/");
  });

  server.on("/music/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    music_led.set_power(PWR_OFF);
    request->redirect("/");
  });

  server.on("/music/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led.get_power()));
  });

  server.on("/music/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led.get_brightness_percent()));
  });
}