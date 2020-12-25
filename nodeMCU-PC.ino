#define debug false
#define FASTLED_INTERNAL
//#define FASTLED_SHOW_CORE 0

#define MUSIC_TRIG 32
#define REDPIN 14
#define REDCHANNEL 1
#define GREENPIN 27
#define GREENCHANNEL 2
#define BLUEPIN 26
#define BLUECHANNEL 3

#include <WiFi.h>
//#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include "rgbdevice.h"

// FastLED
#define DATA_PIN    33
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    24

// Normal LED
#define ANALOG_FREQ 60
// 8-bit
#define ANALOG_RESOLUTION 8
#define ANALOG_HZ 20
#define DIGITAL_HZ 20
#define HUE_UPDATE_HZ 20

// Music LED
#define MUSIC_LED_IGNORE 100
#define MUSIC_LED_HOLD 101
#define MUSIC_LED_RELEASE 102

CRGB leds[NUM_LEDS];

void analogDeskLED(unsigned refreshrate = 0, uint8_t rainbow_hue = 0);
void digitalDeskLED(unsigned refreshrate = 0, uint8_t rainbow_hue = 0);
uint8_t musicLED();
void analogMusicLED();
void digitalMusicLED();
void setAnalogRGB(uint8_t red, uint8_t blue, uint8_t green, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel);
void setAnalogRGB(uint32_t rgb, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel);
void setAllWebServerPages();

// WIFI
char ssid[] = "WAZZUP!!! IoT";
char pass[] = "hackintosh-grass-design-fly-clothing";
AsyncWebServer server(9999);

RGBDevice desk_led = RGBDevice("desk_led");
BrightnessDevice music_led = BrightnessDevice("music_led");

// SETUP

void setup() {
  if (debug) Serial.begin(115200);
  ledcSetup(REDCHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(REDPIN, REDCHANNEL);

  ledcSetup(GREENCHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(GREENPIN, GREENCHANNEL);

  ledcSetup(BLUECHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(BLUEPIN, BLUECHANNEL);

  pinMode(MUSIC_TRIG, INPUT_PULLUP);


  // FastLED 
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setDither(0);

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  // Web Server
  setAllWebServerPages();
  server.begin();
}

void loop() {

  uint8_t music_status = musicLED();
  //uint8_t music_status = MUSIC_LED_IGNORE;
  static uint8_t rainbow_hue = 0;
  unsigned analog_rr = ANALOG_HZ;
  unsigned digital_rr = DIGITAL_HZ;

  // Update Rainbow Hue
  static long hue_rotation_delay = millis();
  if (millis() - hue_rotation_delay > 1000/HUE_UPDATE_HZ) {
    rainbow_hue++;
    hue_rotation_delay = millis();
  }

  // Music
  if (music_status == MUSIC_LED_HOLD) {
    return;
  }

  if (music_status == MUSIC_LED_RELEASE) {
    analog_rr = 0;
    digital_rr = 0;
  }

  // Normal
  analogDeskLED(analog_rr, rainbow_hue);
  digitalDeskLED(digital_rr, rainbow_hue);
}

// Analog Implementation of Desk LED
void analogDeskLED(unsigned refreshrate, uint8_t rainbow_hue) {

  static long delay = millis();
  if (millis() - delay < 1000/refreshrate && refreshrate != 0) {
    return;
  }
  delay = millis();

  // Check Off
  if (!desk_led.get_power()) {
    setAnalogRGB(0, 0, 0, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
    if (debug) Serial.println("Analog OFF");
    return;
  }

  uint32_t desk_led_rgb = desk_led.get_rgb();
  if (debug) Serial.println("desk_led_rgb = " + String(desk_led.get_rgb()));

  // Rainbow
  if (desk_led_rgb == 0xffffff) {
    CRGB rgb;
    hsv2rgb_rainbow(CHSV(rainbow_hue,255,255), rgb);
    setAnalogRGB(rgb.r, rgb.g, rgb.b, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
  }

  // Normal
  else {
    setAnalogRGB(desk_led_rgb, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
  }
}

// Digital Implementation of Desk LED
void digitalDeskLED(unsigned refreshrate, uint8_t rainbow_hue) {

  static uint8_t hue_width = 9;
  static long delay = millis();

  if (millis() - delay < 1000/refreshrate && refreshrate != 0) {
    return;
  }
  delay = millis();

  // Check Off
  if (!desk_led.get_power()) {
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    return;
  }

  uint32_t desk_led_rgb = desk_led.get_rgb();

  // Rainbow
  if (desk_led_rgb == 0xffffff) {
      if (debug) Serial.println("Rainbow digital");
      fill_rainbow(leds, NUM_LEDS, rainbow_hue, hue_width);
  }

  // Normal
  else for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = desk_led_rgb;
  }

  FastLED.show();
}

// Returns true when music overrides normal LED
uint8_t musicLED() {
  if (music_led.get_power() == PWR_ON) {
    static bool last_music;
    bool current_music = !digitalRead(MUSIC_TRIG);

    if (!last_music && !current_music) {
      return MUSIC_LED_IGNORE;
    }

    if (last_music && current_music) {
      return MUSIC_LED_HOLD;
    }

    if (last_music && !current_music) {
      return MUSIC_LED_RELEASE;
    }

    analogMusicLED();
    digitalMusicLED();
  }
  return MUSIC_LED_IGNORE;
}

void analogMusicLED() {
}

void digitalMusicLED() {
}


//
//  HELPER FUNCTIONS
//

void setAnalogRGB(uint8_t red, uint8_t blue, uint8_t green, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel) {
  if (debug) {
    Serial.println("Analog RGB Write: " + String(red) + String(green) + String(blue));
  }
  ledcWrite(red_channel, red); // Red
  ledcWrite(green_channel, green); // Green
  ledcWrite(blue_channel, blue); // Blue
}

void setAnalogRGB(uint32_t rgb, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel) {
  uint8_t r, g, b;
  r = (rgb >> 16) && 0xFF;
  g = (rgb >> 8) && 0xFF;
  b = (rgb) && 0xFF;
  setAnalogRGB(r, g, b, red_channel, green_channel, blue_channel);
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
    response->addHeader("RGB", desk_led.get_rgb_str());
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