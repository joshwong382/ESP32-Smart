#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <HomeSpan.h>
#include "smartdevice.h"
#include "frontend.h"

static bool homespan_init = false;

struct HOMEKIT_ACCESSORY : SpanAccessory {

  HOMEKIT_ACCESSORY(const char *name) : SpanAccessory() {

    if (!homespan_init) {
      homeSpan.begin(Category::Lighting, "NodeMCU-PC", "NodeMCU-PC", "NodeMCU-PC");
      homespan_init = true;
    }

    new Service::AccessoryInformation();
    new Characteristic::Name(name);
    new Characteristic::Manufacturer("joshua@josh-wong.net");
    new Characteristic::SerialNumber("JOSH-207");
    new Characteristic::Model("ESP32");
    new Characteristic::FirmwareRevision("2.3");
    new Characteristic::Identify();

    new Service::HAPProtocolInformation();
    new Characteristic::Version("1.1.0");
  }

  static void loop() {
    if (homespan_init) homeSpan.poll();
  }

};

struct HOMEKIT_RGBLED : Service::LightBulb {

  SpanCharacteristic *power;                   // reference to the On Characteristic
  SpanCharacteristic *H;                       // reference to the Hue Characteristic
  SpanCharacteristic *S;                       // reference to the Saturation Characteristic
  SpanCharacteristic *V;                       // reference to the Brightness Characteristic
  RGBDevice *internalrgbdevice;

  void internal_update() {

    const CRGB fastled_rgb = CRGB(internalrgbdevice->get_rgb());
    const CHSV fastled_hsv = rgb2hsv_approximate(fastled_rgb);

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
      internalrgbdevice->set_power(power->getNewVal(), FrontEnd::HomeKit);
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
    const uint8_t uint_h = uint8_t(new_h * 255 / 360 - 1);
    const uint8_t uint_s = uint8_t(new_s * 255 / 100);
    const uint8_t uint_v = uint8_t(new_v * 255 / 100);
    hsv2rgb_rainbow(CHSV(uint_h, uint_s, uint_v), fastled_rgb);
    if (debug) Serial.println("HomeKit r: " + String(fastled_rgb.r) + " g: " + String(fastled_rgb.g) + " b: " + String(fastled_rgb.b));
    internalrgbdevice->set_rgb(fastled_rgb.red, fastled_rgb.green, fastled_rgb.blue, FrontEnd::HomeKit);
  }

};
