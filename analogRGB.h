#pragma once
#include "smartdevices.h"
#include "smartsensors.h"
#include "driver.h"

#define ANALOG_FREQ 75            // Refresh Rate of PWM (Calculates Duty Cycles)
#define ANALOG_RESOLUTION 8       // # of bits color eg. 8 bits per color (R,G,B) yields -> 16 Million Colors

class AnalogRGB : public MusicDeviceDriver {
    
    private:
        const short unsigned r_ledc;
        const short unsigned g_ledc;
        const short unsigned b_ledc;
        uint32_t rainbow_trigger_color;         // On an RGB color, rainbow instead of displaying that color
        unsigned rainbow_freq;                  // Rainbow Update Refresh Rate

    public:
        AnalogRGB(RGBDevice* _dev, MusicRGBDevice* _musicdev, const uint8_t r_pin, const uint8_t g_pin, const uint8_t b_pin, const uint8_t r_c, const uint8_t g_c, const uint8_t b_c);
        void loop();
        void setRainbowRefreshRate(const unsigned freq);
        void setRainbowColor(const uint32_t color);

    private:
        void RGBLogic(const bool update, const uint8_t rainbow_hue, const bool music);
        void setRGB(const uint8_t red, const uint8_t green, const uint8_t blue);
        void setRGB(const uint32_t rgb);
};
