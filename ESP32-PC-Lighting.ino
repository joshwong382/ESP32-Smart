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
#include "sensitivedata.h"
#include "smartdevices.h"
#include "smartsensors.h"
#include "OpenWeatherMap.h"
#include "webserver.h"
#include "homekit.h"
#include "analogRGB.h"

// FastLED
#define DATA_PIN    33            // Control PIN
#define LED_TYPE    WS2812B       // Programmable Chip Type
#define COLOR_ORDER GRB           // Hardware RGB order
#define NUM_LEDS    24            // # of LEDs per Digital Strip
#define HUE_WIDTH 9               // the width of the rainbow spread in # of LEDs

// Analog LED
#define REDPIN 26                 // Red PWM PIN
#define REDCHANNEL 10             // Red LEDC Channel
#define GREENPIN 27
#define GREENCHANNEL 2
#define BLUEPIN 14
#define BLUECHANNEL 3

// Updates
#define ANALOG_RAINBOW_HZ 20      // How often do we check for change of color (Analog)
#define DIGITAL_RAINBOW_HZ 20
#define HUE_UPDATE_HZ 10          // How fast the rainbow goes in rainbow mode

// Music LED
#define MUSIC_TRIG 32             // Pull to ground to trigger a music specific color

void homekit_loop(const FrontEnd update);
void analogDeskLED(const bool update, const uint8_t rainbow_hue);
void digitalDeskLED(const bool update, const uint8_t rainbow_hue);
const MusicStatus musicLED();
void analogMusicLED();
void digitalMusicLED();
void setAnalogRGB(const uint8_t red, const uint8_t blue, const uint8_t green, const uint8_t red_channel, const uint8_t green_channel, const uint8_t blue_channel);
void setAnalogRGB(const uint32_t rgb, const uint8_t red_channel, const uint8_t green_channel, const uint8_t blue_channel);

//TLC5971 tlc;
CRGB leds[NUM_LEDS];
RGBDevice desk_led = RGBDevice("desk_led");
MusicRGBDevice music_led = MusicRGBDevice("music_led", MUSIC_TRIG);
AnalogRGB analog_rgb = AnalogRGB(&desk_led, &music_led, REDPIN, GREENPIN, BLUEPIN, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
HOMEKIT_RGBLED *desk_led_homekit;
Weather outdoor_weather = Weather("openweathermap");
OpenWeatherMap openweathermap = OpenWeatherMap(&outdoor_weather, sensitive_cityid, sensitive_openweathermap_token);
WebServer server = WebServer(9999, "joshua@josh-wong.net", "ESP32", "JOSH-207");

void setup() {
  Serial.begin(115200);

  // FastLED 
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

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

  MusicStatus music_status = musicLED();
  uint8_t rainbow_hue = music_led.getRainbowHue();

  // Music

  if (music_status == MusicStatus::HOLD) {
    return;
  }

  if (music_status == MusicStatus::RELEASE) {
    digitalDeskLED(true, rainbow_hue);
  }

  // Normal Desk LED Status
  const FrontEnd update_frontend = desk_led.statusChanged(1);
  const bool update = static_cast<bool>(update_frontend);
  digitalDeskLED(update, rainbow_hue);

  homekit_loop(update_frontend);
}

void homekit_loop(const FrontEnd update) {
  HOMEKIT_ACCESSORY::loop();

  if (update == FrontEnd::None) return;
  if (update != FrontEnd::HomeKit) return;
  if (desk_led_homekit == NULL) return;

  desk_led_homekit->internal_update();
}

// Digital Implementation of Desk LED
void digitalDeskLED(const bool update, const uint8_t rainbow_hue) {

  // Check Off
  if (!desk_led.getPower() && update) {
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    if (debug) Serial.println("Digital OFF");
    return;
  }

  const uint32_t desk_led_rgb = desk_led.getRGB();

  // Rainbow
  if (desk_led.getPower() && desk_led_rgb == 0xffffff) {
      EVERY_N_MILLISECONDS(1000/DIGITAL_RAINBOW_HZ) {
        //if (debug) Serial.println("Rainbow digital");
        fill_rainbow(leds, NUM_LEDS, rainbow_hue, HUE_WIDTH);
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
const MusicStatus musicLED() {
  if (music_led.getPower() == PWR_ON) {
    static bool last_music;
    const bool current_music = !digitalRead(MUSIC_TRIG);

    if (!last_music && !current_music) {
      last_music = current_music;
      return MusicStatus::IGNORE;
    }

    if (last_music && current_music) {
      last_music = current_music;
      return MusicStatus::HOLD;
    }

    if (last_music && !current_music) {
      last_music = current_music;
      return MusicStatus::RELEASE;
    }

    // Digital
    for (int i=0; i<NUM_LEDS; i++) {
      leds[i] = CRGB::White;
    }
    FastLED.show();

    last_music = current_music;
    return MusicStatus::HOLD;
  }

  return MusicStatus::IGNORE;
}