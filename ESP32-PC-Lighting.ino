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

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include "secretdata.h"
#include "OpenWeatherMap.h"
#include "webserver.h"
#include "homekit.h"
#include "analogRGB.h"
#include "digitalRGB.h"
#include "SmartManager.h"

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

// Music LED
#define MUSIC_TRIG 32             // Pull to ground to trigger a music specific color

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);

  // Initialize Devices
  RGBDevice* desk_led = new RGBDevice("desk_led");
  MusicRGBDevice* music_led = new MusicRGBDevice("music_led", MUSIC_TRIG);

  // Initialize Sensors
  Weather* outdoor_weather = new Weather("openweathermap");

  // Initialize Drivers
  CLEDController* controller = &(FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip));
  new DigitalRGB(desk_led, music_led, controller);
  new AnalogRGB(desk_led, music_led, REDPIN, GREENPIN, BLUEPIN, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
  new OpenWeatherMap(outdoor_weather, secret_cityid, secret_openweathermap_token);

  // WiFi, Link HomeKit to Devices/Sensors
  // Required to link at least 1 HomeKit Device for HomeSpan's internal WiFi logic
  // see https://github.com/HomeSpan/HomeSpan/blob/master/docs/CLI.md
  WiFi.mode(WIFI_STA);
  new HomeSpanAccessory(desk_led);
  new HomeSpanAccessory(music_led);
  
  // Web Server
  WebServer* server = new WebServer(9999, "joshua@josh-wong.net", "ESP32-PC-Lighting", "JOSH-207");
  server->begin();
}

void loop() {
  SmartManager::loopall();
}