#include <Arduino.h>
#include "analogRGB.h"
#define ANALOG_FREQ 75            // Refresh Rate of PWM (Calculates Duty Cycles)
#define ANALOG_RESOLUTION 8       // # of bits color eg. 8 bits per color (R,G,B) yields -> 16 Million Colors

AnalogRGB::AnalogRGB(RGBDevice* _dev, MusicRGBDevice* _musicdev, const uint8_t r_pin, const uint8_t g_pin, const uint8_t b_pin, const uint8_t r_c, const uint8_t g_c, const uint8_t b_c) :
            r_ledc{r_c}, 
            g_ledc{g_c}, 
            b_ledc{b_c}, 
            MusicDeviceDriver(_dev, _musicdev) {
    ledcSetup(r_ledc, ANALOG_FREQ, ANALOG_RESOLUTION);
    ledcAttachPin(r_pin, r_ledc);

    ledcSetup(g_ledc, ANALOG_FREQ, ANALOG_RESOLUTION);
    ledcAttachPin(g_pin, g_ledc);

    ledcSetup(b_ledc, ANALOG_FREQ, ANALOG_RESOLUTION);
    ledcAttachPin(b_pin, b_ledc);

    rainbow_freq = 20;
    rainbow_trigger_color = 0xffffff;
}

void AnalogRGB::loop() {
    MusicStatus music_status = musicdev->musicStatus();
    const uint8_t rainbow_hue = musicdev->getRainbowHue();

    if (music_status == MusicStatus::HOLD) {
        return;
    }

    if (music_status == MusicStatus::RELEASE) {
        RGBLogic(true, rainbow_hue);
    }

    const FrontEnd update_frontend = dev->statusChanged(0);
    const bool update = static_cast<bool>(update_frontend);
    RGBLogic(update, rainbow_hue);
}

void AnalogRGB::RGBLogic(const bool update, const uint8_t rainbow_hue) {

    if (!dev->getPower() && update) {
        // Off
        setRGB(0, 0, 0);
        return;
    }

    uint32_t rgb = ((RGBDevice*) dev)->getRGB();

    // Rainbow
    
    if (dev->getPower() && rgb == rainbow_trigger_color) {
        EVERY_N_MILLISECONDS(1000/rainbow_freq) {
            CRGB rainbow_rgb;
            hsv2rgb_rainbow(CHSV(rainbow_hue,255,255), rainbow_rgb);
            setRGB(rainbow_rgb.r, rainbow_rgb.g, rainbow_rgb.b);
        }
        return;
    }

    // Non-Rainbow
    if (update) {
        setRGB(rgb);
    }
}

void AnalogRGB::setRGB(const uint8_t red, const uint8_t green, const uint8_t blue) {
    ledcWrite(r_ledc, red);
    ledcWrite(g_ledc, green);
    ledcWrite(b_ledc, blue);

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

void AnalogRGB::setRGB(const uint32_t rgb) {
    uint8_t r, g, b;
    r = (rgb >> 16) & 0xFF;
    g = (rgb >>  8) & 0xFF;
    b = rgb & 0xFF;
    setRGB(r, g, b);
}