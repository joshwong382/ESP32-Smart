#define debug 0

// FastLED Core Distribution
//#define FASTLED_ALLOW_INTERRUPTS 0
//#define CONFIG_ASYNC_TCP_RUNNING_CORE 1
//#define FASTLED_SHOW_CORE 0

// FastLED defaults to using the RMT for timing.
// https://github.com/FastLED/FastLED/blob/master/src/platforms/esp/32/clockless_rmt_esp32.h
//#define FASTLED_RMT_MAX_CHANNELS 4
//#define FASTLED_ESP32_FLASH_LOCK 1

// FastLED switch to I2S or SPI
// SPI functionality in this fork: https://github.com/ntwallace/FastLED/blob/master/platforms/esp/32/fastspi_esp32.h
//#define FASTLED_ESP32_I2S 1
//#define FASTLED_ALL_PINS_HARDWARE_SPI 1
//#define FASTLED_ESP32_SPI_BUS HSPI 1

//#include "tlc57911.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include "secretdata.h"
#include "smartdevices.h"
#include "smartsensors.h"
#include "OpenWeatherMap.h"
#include "webserver.h"
#include "homekit.h"
#include "analogRGB.h"
#include "digitalRGB.h"

// FastLED
#define DATA_PIN    33            // Control PIN
#define LED_TYPE    WS2812B       // Programmable Chip Type
#define COLOR_ORDER GRB           // Hardware RGB order
#define NUM_LEDS    24            // # of LEDs per Digital Strip

// Analog LED
#define REDPIN 26                 // Red PWM PIN
#define REDCHANNEL 10             // Red LEDC Channel
#define GREENPIN 27
#define GREENCHANNEL 2
#define BLUEPIN 14
#define BLUECHANNEL 3

// Updates
#define HUE_UPDATE_HZ 10          // How fast the rainbow goes in rainbow mode

// Music LED
#define MUSIC_TRIG 32             // Pull to ground to trigger a music specific color

void homekit_loop(const FrontEnd update);

CRGB leds[NUM_LEDS];

RGBDevice desk_led = RGBDevice("desk_led");
MusicRGBDevice music_led = MusicRGBDevice("music_led", MUSIC_TRIG);

AnalogRGB analog_rgb = AnalogRGB(&desk_led, &music_led, REDPIN, GREENPIN, BLUEPIN, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
DigitalRGB *digital_rgb;

HOMEKIT_RGBLED *desk_led_homekit;

Weather outdoor_weather = Weather("openweathermap");
OpenWeatherMap openweathermap = OpenWeatherMap(&outdoor_weather, secret_cityid, secret_openweathermap_token);

WebServer server = WebServer(9999, "joshua@josh-wong.net", "ESP32", "JOSH-207");

void setup() {
  Serial.begin(115200);

  // FastLED 
  CLEDController* controller = &(FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip));
  digital_rgb = new DigitalRGB(&desk_led, &music_led, controller);

  // Wi-Fi
  WiFi.mode(WIFI_STA);

  // Homespan
  new HOMEKIT_ACCESSORY("desk_led");
  desk_led_homekit = new HOMEKIT_RGBLED(&desk_led);
  
  // Web Server
  smartDevices.add(&desk_led);
  smartDevices.add(&music_led);
  smartSensors.add(&outdoor_weather);
  server.begin();
}

void loop() {
  openweathermap.loop();
  music_led.loop();
  analog_rgb.loop();
  digital_rgb->loop();

  // Normal Desk LED Status
  const FrontEnd update_frontend = desk_led.statusChanged(2);
  const bool update = static_cast<bool>(update_frontend);
  homekit_loop(update_frontend);
}

void homekit_loop(const FrontEnd update) {
  HOMEKIT_ACCESSORY::loop();

  if (update == FrontEnd::None) return;
  if (update != FrontEnd::HomeKit) return;
  if (desk_led_homekit == NULL) return;

  desk_led_homekit->internal_update();
}