#define debug false
//#define FASTLED_INTERNAL
//#define FASTLED_ALLOW_INTERRUPTS 0
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define FASTLED_SHOW_CORE 1
//#define FASTLED_ESP32_I2S
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <HomeSpan.h>
//#include "tlc57911.h"
#include "rgbdevice.h"

// FastLED
#define DATA_PIN    33
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    24

// Analog LED
#define ANALOG_FREQ 60
#define ANALOG_RESOLUTION 8
#define MUSIC_TRIG 32
#define REDPIN 26
#define REDCHANNEL 10
#define GREENPIN 27
#define GREENCHANNEL 2
#define BLUEPIN 14
#define BLUECHANNEL 3

// Updates
#define ANALOG_RAINBOW_HZ 20
#define DIGITAL_RAINBOW_HZ 20
#define HUE_UPDATE_HZ 10

// Music LED
#define MUSIC_LED_IGNORE 100
#define MUSIC_LED_HOLD 101
#define MUSIC_LED_RELEASE 102

CRGB leds[NUM_LEDS];

void analogDeskLED(bool update, uint8_t rainbow_hue);
void digitalDeskLED(bool update, uint8_t rainbow_hue);
uint8_t musicLED();
void analogMusicLED();
void digitalMusicLED();
void setAnalogRGB(uint8_t red, uint8_t blue, uint8_t green, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel);
void setAnalogRGB(uint32_t rgb, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel);

void setAllWebServerPages();
String ip2String(const IPAddress& ipAddress);
bool isValidIP(const IPAddress& ipAddress);
//bool wifi_check();

// HomeKit
struct HOMEKIT_RGBLED : Service::LightBulb {

  SpanCharacteristic *power;                   // reference to the On Characteristic
  SpanCharacteristic *H;                       // reference to the Hue Characteristic
  SpanCharacteristic *S;                       // reference to the Saturation Characteristic
  SpanCharacteristic *V;                       // reference to the Brightness Characteristic
  RGBDevice *internalrgbdevice;

  void internal_update() {

    CRGB fastled_rgb = CRGB(internalrgbdevice->get_rgb());
    CHSV fastled_hsv = rgb2hsv_approximate(fastled_rgb);

    power->setVal(internalrgbdevice->get_power());
    H->setVal(fastled_hsv.hue * 360 / 255);
    S->setVal(fastled_hsv.saturation * 100 / 255);
    V->setVal(fastled_hsv.value * 100 / 255);
  }

  HOMEKIT_RGBLED(RGBDevice *_internaldev) : Service::LightBulb() {
    internalrgbdevice = _internaldev;
    power = new Characteristic::On();
    H = new Characteristic::Hue();
    S = new Characteristic::Saturation();
    V = new Characteristic::Brightness();
    V->setRange(1, 100, 1); // min 1%, max 100%, in 1% increments

    internal_update();
  }

  boolean update() {

    // H[0-360]
    // S[0-100]
    // V[0-100]

    float new_h = H->getVal<float>();
    float new_s = S->getVal<float>();
    float new_v = V->getVal<float>();

    if (power->updated()) {
      internalrgbdevice->set_power(power->getNewVal(), Device::HomeKit);
    }

    if (H->updated()) {
      new_h = H->getNewVal();
    }

    if (S->updated()) {
      new_s = S->getNewVal();
    }

    if (V->updated()) {
      new_v = V->getNewVal();
    }

    if (debug) Serial.println("HomeKit H: " + String(new_h) + " S: " + String(new_s) + " V: " + String(new_v));

    CRGB fastled_rgb;
    uint8_t uint_h = uint8_t(new_h * 255 / 360 - 1);
    uint8_t uint_s = uint8_t(new_s * 255 / 100);
    uint8_t uint_v = uint8_t(new_v * 255 / 100);
    hsv2rgb_rainbow(CHSV(uint_h, uint_s, uint_v), fastled_rgb);
    if (debug) Serial.println("HomeKit r: " + String(fastled_rgb.r) + " g: " + String(fastled_rgb.g) + " b: " + String(fastled_rgb.b));
    internalrgbdevice->set_rgb(fastled_rgb.red, fastled_rgb.green, fastled_rgb.blue, Device::HomeKit);
  }

};

//TLC5971 tlc;
RGBDevice desk_led = RGBDevice("desk_led");
HOMEKIT_RGBLED *desk_led_homekit;
BrightnessDevice music_led = BrightnessDevice("music_led");
AsyncWebServer server(9999);

// SETUP
void homespan_setup() {
  homeSpan.begin(Category::Lighting, "NodeMCU-PC", "NodeMCU-PC-", "NodeMCU-PC");

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Name("Desk LED");
  new Characteristic::Manufacturer("joshua@josh-wong.net");
  new Characteristic::SerialNumber("JOSH-207");
  new Characteristic::Model("ESP32");
  new Characteristic::FirmwareRevision("2.3");
  new Characteristic::Identify();

  new Service::HAPProtocolInformation();
  new Characteristic::Version("1.1.0");
  desk_led_homekit = new HOMEKIT_RGBLED(&desk_led);
}

void setup() {
  Serial.begin(115200);
  
  // Analog
  ledcSetup(REDCHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(REDPIN, REDCHANNEL);

  ledcSetup(GREENCHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(GREENPIN, GREENCHANNEL);

  ledcSetup(BLUECHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(BLUEPIN, BLUECHANNEL);
  //if (debug) Serial.println("Setting up TLC57911...");
  //tlc.initializeTLC();
  
  pinMode(MUSIC_TRIG, INPUT_PULLUP);

  // FastLED 
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Wi-Fi
  WiFi.mode(WIFI_STA);

  // Homespan
  homespan_setup();  
  
  // Web Server
  setAllWebServerPages();
  server.begin();
}

void loop() {

  homeSpan.poll();

  uint8_t music_status = musicLED();
  static uint8_t rainbow_hue = 0;

  // Update Rainbow Hue
  EVERY_N_MILLISECONDS(1000/HUE_UPDATE_HZ) {
    rainbow_hue++;
  }

  // Music
  if (music_status == MUSIC_LED_HOLD) {
    return;
  }

  if (music_status == MUSIC_LED_RELEASE) {
    analogDeskLED(true, rainbow_hue);
    digitalDeskLED(true, rainbow_hue);
  }

  // Normal Desk LED Status
  Device::Devices deskled_status_changed = desk_led.status_changed();
  digitalDeskLED(deskled_status_changed, rainbow_hue);
  analogDeskLED(deskled_status_changed, rainbow_hue);
  if (deskled_status_changed != Device::None)
    if (deskled_status_changed != Device::HomeKit)
      if (desk_led_homekit != NULL) desk_led_homekit->internal_update();
}

// Analog Implementation of Desk LED
void analogDeskLED(bool update, uint8_t rainbow_hue) {

  // Check Off
  if (!desk_led.get_power() && update) {
    setAnalogRGB(0, 0, 0, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
    if (debug) Serial.println("Analog OFF");
    return;
  }

  uint32_t desk_led_rgb = desk_led.get_rgb();

  // Rainbow
  if (desk_led.get_power() && desk_led_rgb == 0xffffff) {
    EVERY_N_MILLISECONDS(1000/ANALOG_RAINBOW_HZ) {
      CRGB rgb;
      hsv2rgb_rainbow(CHSV(rainbow_hue,255,255), rgb);
      setAnalogRGB(rgb.r, rgb.g, rgb.b, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
    }
    return;
  }

  // Normal
  if (update) {
    setAnalogRGB(desk_led_rgb, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
  }
}

// Digital Implementation of Desk LED
void digitalDeskLED(bool update, uint8_t rainbow_hue) {

  static uint8_t hue_width = 9;

  // Check Off
  if (!desk_led.get_power() && update) {
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    if (debug) Serial.println("Digital OFF");
    return;
  }

  uint32_t desk_led_rgb = desk_led.get_rgb();

  // Rainbow
  if (desk_led.get_power() && desk_led_rgb == 0xffffff) {
      EVERY_N_MILLISECONDS(1000/DIGITAL_RAINBOW_HZ) {
        //if (debug) Serial.println("Rainbow digital");
        fill_rainbow(leds, NUM_LEDS, rainbow_hue, hue_width);
        FastLED.show();
      }
      return;
  }

  // Normal
  if (update) {
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = desk_led_rgb;
    }
    FastLED.show();
  }
  
}

// Returns true when music overrides normal LED
uint8_t musicLED() {
  if (music_led.get_power() == PWR_ON) {
    static bool last_music;
    bool current_music = !digitalRead(MUSIC_TRIG);

    if (!last_music && !current_music) {
      last_music = current_music;
      return MUSIC_LED_IGNORE;
    }

    if (last_music && current_music) {
      last_music = current_music;
      return MUSIC_LED_HOLD;
    }

    if (last_music && !current_music) {
      last_music = current_music;
      return MUSIC_LED_RELEASE;
    }

    // Digital
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB::White;
    }
    FastLED.show();

    // Analog
    setAnalogRGB(255, 255, 255, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);

    last_music = current_music;
    return MUSIC_LED_HOLD;
  }

  return MUSIC_LED_IGNORE;
}

//
//  HELPER FUNCTIONS
//

void setAnalogRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel) {
  //if (debug) Serial.println("Analog RGB Write: r" + String(red, HEX) + " g" + String(green, HEX) + " b" + String(blue, HEX));

  ledcWrite(red_channel, red); // Red
  ledcWrite(green_channel, green); // Green
  ledcWrite(blue_channel, blue); // Blue
  
  /*
  // Convert to 16 bit color
  uint16_t red_16 = (red + 1) * 256 - 1;
  uint16_t green_16 = (green + 1) * 256 - 1;
  uint16_t blue_16 = (blue + 1) * 256 - 1;
  
  for(uint16_t i=0; i < 3 * NUM_TLC59711 - 1; i++) {
      tlc.setLED(i*3, red_16);
      tlc.setLED(i*3+1, green_16);
      tlc.setLED(i*3+2, blue_16);
      tlc.writeLED();
  }
  */
  
}

void setAnalogRGB(uint32_t rgb, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel) {
  uint8_t r, g, b;
  r = (rgb >> 16) & 0xFF;
  g = (rgb >>  8) & 0xFF;
  b = rgb & 0xFF;
  setAnalogRGB(r, g, b, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
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
    
    desk_led.set_power(PWR_ON, Device::HTTP_API);

    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      char color_arr[7];
      color.toCharArray(color_arr, 7);
      uint32_t rgb = strtol(color_arr, NULL, 16);
      desk_led.set_rgb(rgb, Device::HTTP_API);
      
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
        desk_led.set_brightness_percent(brightness, Device::HTTP_API);
      }
    }

    request->redirect("/");
  });

  server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    desk_led.set_power(PWR_OFF, Device::HTTP_API);
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
        music_led.set_brightness_percent(brightness, Device::HTTP_API);
      }
    }
    
    request->redirect("/");
  });

  server.on("/music/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    music_led.set_power(PWR_OFF, Device::HTTP_API);
    request->redirect("/");
  });

  server.on("/music/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led.get_power()));
  });

  server.on("/music/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(music_led.get_brightness_percent()));
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