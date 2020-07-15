#define ESPALEXA_ASYNC
#define FASTLED_INTERNAL

#define REDPIN D6
#define GREENPIN D7
#define BLUEPIN D8
#define MUSIC_TRIG D2

#include <ESP8266WiFi.h>
#include <Espalexa.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <RGBConverter.h>
#include "helper.h"

// FastLED
#define DATA_PIN    5
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    24
#define FRAMES_PER_SECOND  120
CRGB leds[NUM_LEDS];
uint8_t gHue = 0; // rotating "base color" on FastLED

// WIFI
char ssid[] = "***REMOVED******REMOVED***";
char pass[] = "***REMOVED***";
AsyncWebServer server(9999);
Espalexa espalexa;
EspalexaDevice* desk_led;
EspalexaDevice* music_led;

void setup() {
  Serial.begin(115200);
  analogWriteRange(255);
  pinMode(MUSIC_TRIG, INPUT_PULLUP);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  // FastLED
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  wifi_check(ssid, pass);

  // ESPAlexa
  desk_led = new EspalexaDevice("Desk LED", deskLED_callback, 0);
  music_led = new EspalexaDevice("Music LED", musicLED_callback, 102);
  espalexa.addDevice(desk_led);
  espalexa.addDevice(music_led);
  espalexa.begin();

  // Web Server
  setAllWebServerPages();
  server.begin();
}

void loop() {

  // Check Wifi
  wifi_check(ssid, pass);
  espalexa.loop();
  doRGB();
}

void doRGB() {

  static bool last_music;

  // Digital LEDs
  if (music_led->getValue() && !digitalRead(MUSIC_TRIG)) { // music led

    // Analog
    setAnalogRGB(255, 255, 255, music_led->getLastValue(), REDPIN, GREENPIN, BLUEPIN);

    // Digital
    for (int i=0; i<NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    }

    // check if delay is needed
    if (last_music) FastLED.delay(1000/FRAMES_PER_SECOND);
    last_music = true;
    FastLED.setBrightness(music_led->getLastValue());
    FastLED.show();
    
  } else if (desk_led->getValue()) { // normal led

    // check if delay is needed
    if (!last_music) FastLED.delay(1000/FRAMES_PER_SECOND);
    last_music = false;
    // Digital if white then rainbow
    if (desk_led->getRGB() == 0xffffff) {
      fill_rainbow(leds, NUM_LEDS, gHue, 8); //digital
      CRGB rgb;
      hsv2rgb_rainbow(CHSV(gHue,255,255), rgb);
      setAnalogRGB(rgb, desk_led->getValue(), REDPIN, GREENPIN, BLUEPIN); // analog
    }
    else {
      uint16_t h = desk_led->getHue();
      uint8_t s = desk_led->getSat();
      uint8_t v = desk_led->getValue();
      for (int i=0; i<NUM_LEDS; i++) {
        leds[i] = CHSV(h*255/65535, s, v);
      }
      RGBConverter cvt;
      byte rgb[3];
      cvt.hsvToRgb(double(h)/65535, double(s)/255, double(v)/255, rgb);
      setAnalogRGB(rgb[0], rgb[2], rgb[1], 255, REDPIN, GREENPIN, BLUEPIN);
    }
    FastLED.setBrightness(desk_led->getLastValue());
    FastLED.show();
    EVERY_N_MILLISECONDS( 50 ) { gHue++; }
    
  } else { // off

    // Analog
    setAnalogRGB(0, 0, 0, 0, REDPIN, GREENPIN, BLUEPIN);

    // Digital
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    
  }
}

// Web Server
void setAllWebServerPages() {

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String read1 = desk_led->getValue() ? String(rgbtopercent(desk_led->getValue())) + "%" : "OFF";
    String read2 = desk_led->getValue() ? "off" : "on";
    String read3 = music_led->getValue() ? String(rgbtopercent(music_led->getValue())) + "%" : "OFF";
    String read4 = music_led->getValue() ? "off" : "on";
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
    response_html += "\ndesk_led " + String(rgbtopercent(desk_led->getValue()));
    response_html += "\nmusic_led " + String(rgbtopercent(music_led->getValue()));
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", response_html);
    request->send(response);
  });

  server.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      char color_arr[7];
      color.toCharArray(color_arr, 7);
      uint32_t rgb = strtol(color_arr, NULL, 16);
      uint8_t r = (rgb >> 16) & 0xFF;
      uint8_t g = (rgb >>  8) & 0xFF;
      uint8_t b = rgb & 0xFF;
      
      double hsv[3];
      RGBConverter cvt;
      cvt.rgbToHsv(r, g, b, hsv);
      uint8_t br = hsv[2]*255;
      uint8_t sat = hsv[1]*255;
      uint16_t hue = hsv[0]*65535;
      
      desk_led->setColor(hue, sat);
      desk_led->setValue(br);
      deskLED_callback(br, desk_led->getRGB());
      
      AsyncWebServerResponse *response = request->beginResponse(302);
      response->addHeader("RGB: R: ", colorinttohexstr(r) + "G: " + colorinttohexstr(g) + "B: " + colorinttohexstr(b));
      response->addHeader("HSV", String(hue) + "," + String(sat) + "," + String(br));
      response->addHeader("Location", "/");
      request->send(response);
      return;
    }

    int brightness = -1;
    if (request->hasParam("brightness")) {
      brightness = request->getParam("brightness")->value().toInt();
      brightness = percenttorgb(brightness);
      if (brightness <= 0 || brightness >= 256) {
        request->redirect("/");
      }
    } else {
      brightness = desk_led->getLastValue();
    }
    
    desk_led->setValue(brightness);
    deskLED_callback(brightness, desk_led->getRGB());
    request->redirect("/");
  });

  server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    desk_led->setValue(0);
    deskLED_callback(0, desk_led->getRGB());
    request->redirect("/");
  });

  server.on("/led/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(desk_led->getValue() ? 1 : 0));
  });

  server.on("/led/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(rgbtopercent(desk_led->getValue())));
  });

  server.on("/led/color", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    //uint32_t rgb = desk_led->getColorRGB();
    //uint8_t r = (rgb >> 16) & 0xFF;
    //uint8_t g = (rgb >>  8) & 0xFF;
    //uint8_t b = rgb & 0xFF;

    uint16_t h = desk_led->getHue();
    uint8_t s = desk_led->getSat();
    uint8_t v = desk_led->getValue();
    
    String hsv = String(h) + " " + String(s) + " " + String(v);
    RGBConverter cvt;
    byte rgb[3];
    cvt.hsvToRgb(round(double(h)/65535), round(double(s)/255), round(double(v)/255), rgb);
    String color = colorinttohexstr(rgb[0]) + colorinttohexstr(rgb[1]) + colorinttohexstr(rgb[2]);
    
    request->send(200, "text/plain", color);
  });

  server.on("/music/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    int brightness = -1;
    if (request->hasParam("brightness")) {
      brightness = request->getParam("brightness")->value().toInt();
      brightness = percenttorgb(brightness);
      if (brightness <= 0 || brightness >= 256) {
        request->redirect("/");
      }
    } else {
      brightness = music_led->getLastValue();
    }
    
    music_led->setValue(brightness);
    musicLED_callback(brightness);
    request->redirect("/");
  });

  server.on("/music/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    music_led->setValue(0);
    musicLED_callback(0);
    request->redirect("/");
  });

  server.on("/music/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led->getValue() ? 1 : 0));
  });

  server.on("/music/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(rgbtopercent(music_led->getValue())));
  });
}