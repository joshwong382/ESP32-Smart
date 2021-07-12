#include <Arduino.h>
#include "analogRGB.h"

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

    // Defaults
    rainbow_freq = 20;
    rainbow_trigger_color = 0xffffff;
}

// Same code as DigitalRGB but shouldn't share the same function
void AnalogRGB::loop() {
    MusicStatus music_status = musicdev->music_status;
    const uint8_t rainbow_hue = musicdev->getRainbowHue();

    switch (music_status) {
        // Apply music color
        case MusicStatus::INITIATE:
            RGBLogic(true, rainbow_hue, true);
            return;

        // Apply non-music color without notifying
        case MusicStatus::RELEASE:
            RGBLogic(true, rainbow_hue, false);
            return;
    
        case MusicStatus::HOLD:
            return;

        default:
            // We don't care about update notification
            const bool update = static_cast<bool>(dev->statusChanged(0));
            RGBLogic(update, rainbow_hue, false);
    }
}

void AnalogRGB::setRainbowRefreshRate(const unsigned freq) {
    rainbow_freq = freq;
}

void AnalogRGB::setRainbowColor(const uint32_t color) {
    rainbow_trigger_color = color;
}

void AnalogRGB::RGBLogic(const bool update, const uint8_t rainbow_hue, const bool music) {

    // Music overrides RGB power (Control with musicRGB power)
    if (music) {
        if (musicdev == NULL) return;
        setRGB(musicdev->getRGB());
        return;
    }

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