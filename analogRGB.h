#pragma once
#include "smartdevices.h"
#include "smartsensors.h"
#include "driver.h"

// ESP32 ONLY!
// Add #ifdef esp32 specific stuff

class AnalogRGB : public MusicDeviceDriver {
    
    private:
        const short unsigned r_ledc;
        const short unsigned g_ledc;
        const short unsigned b_ledc;
        uint32_t rainbow_trigger_color;
        unsigned rainbow_freq;
        void RGBLogic(const bool update, const uint8_t rainbow_hue);

    public:
        AnalogRGB(RGBDevice* _dev, MusicRGBDevice* _musicdev, const uint8_t r_pin, const uint8_t g_pin, const uint8_t b_pin, const uint8_t r_c, const uint8_t g_c, const uint8_t b_c);
        void loop();
        //void setRainbowRefreshRate(const unsigned freq);

    private:
        void setRGB(const uint8_t red, const uint8_t green, const uint8_t blue);
        void setRGB(const uint32_t rgb);


};
