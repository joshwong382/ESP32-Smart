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

// FastLED
#define DATA_PIN    33            // Control PIN
#define LED_TYPE    WS2812B       // Programmable Chip Type
#define COLOR_ORDER GRB           // Hardware RGB order
#define NUM_LEDS    24            // # of LEDs per Digital Strip
#define HUE_WIDTH 9               // the width of the rainbow spread in # of LEDs

// Analog LED
#define ANALOG_FREQ 75            // Refresh Rate of PWM (Calculates Duty Cycles)
#define ANALOG_RESOLUTION 8       // # of bits color eg. 8 bits per color (R,G,B) yields -> 16 Million Colors
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
#define MUSIC_LED_IGNORE 100
#define MUSIC_LED_HOLD 101
#define MUSIC_LED_RELEASE 102

void homekit_loop(const FrontEnd update);
void analogDeskLED(const bool update, const uint8_t rainbow_hue);
void digitalDeskLED(const bool update, const uint8_t rainbow_hue);
const uint8_t musicLED();
void analogMusicLED();
void digitalMusicLED();
void setAnalogRGB(const uint8_t red, const uint8_t blue, const uint8_t green, const uint8_t red_channel, const uint8_t green_channel, const uint8_t blue_channel);
void setAnalogRGB(const uint32_t rgb, const uint8_t red_channel, const uint8_t green_channel, const uint8_t blue_channel);

//TLC5971 tlc;
CRGB leds[NUM_LEDS];
RGBDevice desk_led = RGBDevice("desk_led");
HOMEKIT_RGBLED *desk_led_homekit;
BrightnessDevice music_led = BrightnessDevice("music_led");
Weather outdoor_weather = Weather("openweathermap");
OpenWeatherMap openweathermap = OpenWeatherMap(&outdoor_weather, sensitive_cityid, sensitive_openweathermap_api);
WebServer server = WebServer(9999, "joshua@josh-wong.net", "ESP32", "JOSH-207");

void setup() {
  Serial.begin(115200);
  
  // Analog
  ledcSetup(REDCHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(REDPIN, REDCHANNEL);

  ledcSetup(GREENCHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(GREENPIN, GREENCHANNEL);

  ledcSetup(BLUECHANNEL, ANALOG_FREQ, ANALOG_RESOLUTION);
  ledcAttachPin(BLUEPIN, BLUECHANNEL);
  
  pinMode(MUSIC_TRIG, INPUT_PULLUP);

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

  uint8_t music_status = musicLED();
  static uint8_t rainbow_hue = 0;

  // Update Hue for Rainbow
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
  const FrontEnd update_frontend = desk_led.statusChanged();
  const bool update = static_cast<bool>(update_frontend);
  digitalDeskLED(update, rainbow_hue);
  analogDeskLED(update, rainbow_hue);
  homekit_loop(update_frontend);
}

void homekit_loop(const FrontEnd update) {
  HOMEKIT_ACCESSORY::loop();

  if (update == FrontEnd::None) return;
  if (update != FrontEnd::HomeKit) return;
  if (desk_led_homekit == NULL) return;

  desk_led_homekit->internal_update();
}

// Analog Implementation of Desk LED
void analogDeskLED(const bool update, const uint8_t rainbow_hue) {

  // Check Off
  if (!desk_led.getPower() && update) {
    setAnalogRGB(0, 0, 0, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
    if (debug) Serial.println("Analog OFF");
    return;
  }

  uint32_t desk_led_rgb = desk_led.getRGB();

  // Rainbow
  if (desk_led.getPower() && desk_led_rgb == 0xffffff) {
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
const uint8_t musicLED() {
  if (music_led.getPower() == PWR_ON) {
    static bool last_music;
    const bool current_music = !digitalRead(MUSIC_TRIG);

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

void setAnalogRGB(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t red_channel, const uint8_t green_channel, const uint8_t blue_channel) {
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

void setAnalogRGB(const uint32_t rgb, const uint8_t red_channel, const uint8_t green_channel, const uint8_t blue_channel) {
  uint8_t r, g, b;
  r = (rgb >> 16) & 0xFF;
  g = (rgb >>  8) & 0xFF;
  b = rgb & 0xFF;
  setAnalogRGB(r, g, b, REDCHANNEL, GREENCHANNEL, BLUECHANNEL);
}