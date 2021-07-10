#pragma once
#include <FastLED.h>
#include "smartdevices.h"
#include "smartsensors.h"
#include "driver.h"

// ESP32 ONLY!
// Add #ifdef esp32 specific stuff

class DigitalRGB : public MusicDeviceDriver {
    
    private:
        uint32_t rainbow_trigger_color;
        unsigned rainbow_freq;
        CLEDController* controller;

    public:
        DigitalRGB(RGBDevice* _dev, MusicRGBDevice* _musicdev, CLEDController* const led_controller);
        void loop();
        //void setRainbowRefreshRate(const unsigned freq);

    private:
        void RGBLogic(const bool update, const uint8_t rainbow_hue, const bool music);
        void setRainbow(const uint8_t rainbow_hue);
        void setSingleColor(const CRGB color);
        void setSingleColor(const uint8_t red, const uint8_t green, const uint8_t blue);
        void setSingleColor(const uint32_t rgb);
        void show();
};
