#pragma once
#include <FastLED.h>
#include "../state/smartdevices.h"
#include "../state/smartsensors.h"
#include "../driver.h"

// ESP32 ONLY!
// Add #ifdef esp32 specific stuff

class DigitalRGB : public MusicDeviceDriver {
    
    private:
        uint32_t rainbow_trigger_color;     // On an RGB color, rainbow instead of displaying that color
        unsigned rainbow_freq;              // Rainbow Update Refresh Rate
        uint8_t hue_width;                  // the width of the rainbow spread in # of LEDs
        CLEDController* controller;

    public:
        DigitalRGB(RGBDevice* _dev, MusicRGBDevice* _musicdev, CLEDController* const led_controller);
        void loop();
        void setRainbowRefreshRate(const unsigned freq);
        void setRainbowColor(const uint32_t color);
        void setHueWidth(const uint8_t hw);

    private:
        void RGBLogic(const bool update, const uint8_t rainbow_hue, const bool music);
        void setRainbow(const uint8_t rainbow_hue);
        void setSingleColor(const CRGB color);
        void setSingleColor(const uint8_t red, const uint8_t green, const uint8_t blue);
        void setSingleColor(const uint32_t rgb);
        void show();
};
