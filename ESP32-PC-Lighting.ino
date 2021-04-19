#define debug false
//#define FASTLED_INTERNAL
//#define FASTLED_ALLOW_INTERRUPTS 0
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define FASTLED_SHOW_CORE 1
//#define FASTLED_ESP32_I2S
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI

//#include "tlc57911.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <HomeSpan.h>
#include "rgbdevice.h"
#include "webserver.h"
#include "homekit.h"

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

void analogDeskLED(bool update, uint8_t rainbow_hue);
void digitalDeskLED(bool update, uint8_t rainbow_hue);
uint8_t musicLED();
void analogMusicLED();
void digitalMusicLED();
void setAnalogRGB(uint8_t red, uint8_t blue, uint8_t green, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel);
void setAnalogRGB(uint32_t rgb, uint8_t red_channel, uint8_t green_channel, uint8_t blue_channel);

//void setAllWebServerPages();
//String ip2String(const IPAddress& ipAddress);
//bool isValidIP(const IPAddress& ipAddress);
//bool wifi_check();

//TLC5971 tlc;
CRGB leds[NUM_LEDS];
RGBDevice desk_led = RGBDevice("desk_led");
HOMEKIT_RGBLED *desk_led_homekit;
BrightnessDevice music_led = BrightnessDevice("music_led");
AsyncWebServer server(9999);

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
  homeSpan.begin(Category::Lighting, "NodeMCU-PC", "NodeMCU-PC", "NodeMCU-PC");
  new HOMEKIT_ACCESSORY("desk_led");
  desk_led_homekit = new HOMEKIT_RGBLED(&desk_led);
  
  // Web Server
  setAllWebServerPages(&server, &desk_led, &music_led);
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