#include <Arduino.h>
#include "digitalRGB.h"            

DigitalRGB::DigitalRGB(RGBDevice* _dev, MusicRGBDevice* _musicdev, CLEDController* const _controller) : 
            MusicDeviceDriver(_dev, _musicdev) {

    controller = _controller;

    // Defaults
    rainbow_freq = 20;
    hue_width = 9;
    rainbow_trigger_color = 0xffffff;
}

// Same code as AnalogRGB but shouldn't share the same function
void DigitalRGB::loop() {
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
            const bool update = static_cast<bool>(dev->statusChanged(1));
            RGBLogic(update, rainbow_hue, false);
    }
}

void DigitalRGB::setRainbowRefreshRate(const unsigned freq) {
    rainbow_freq = freq;
}

void DigitalRGB::setRainbowColor(const uint32_t color) {
    rainbow_trigger_color = color;
}

void DigitalRGB::setHueWidth(const uint8_t hw) {
    hue_width = hw;
}

void DigitalRGB::RGBLogic(const bool update, const uint8_t rainbow_hue, const bool music) {

    // Music overrides RGB power (Control with musicRGB power)
    if (music) {
        if (musicdev == NULL) return;
        setSingleColor(musicdev->getRGB());
        return;
    }

    if (!dev->getPower() && update) {
        // Off
        setSingleColor(CRGB::Black);
        return;
    }

    uint32_t rgb = ((RGBDevice*) dev)->getRGB();

    // Rainbow
    if (dev->getPower() && rgb == rainbow_trigger_color) {
        EVERY_N_MILLISECONDS(1000/rainbow_freq) {
            setRainbow(rainbow_hue);
        }
        return;
    }

    // Non-Rainbow
    if (update) {
        setSingleColor(rgb);
    }
}

void DigitalRGB::setRainbow(const uint8_t rainbow_hue) {
    if (controller == NULL) return;

    fill_rainbow(controller->leds(), controller->size(), rainbow_hue, hue_width);
    show();
}

void DigitalRGB::setSingleColor(const CRGB color) {
    if (controller == NULL) return;

    for (int i=0; i<controller->size(); i++) {
      (*controller)[i] = color;
    }
    show();
}

void DigitalRGB::setSingleColor(const uint8_t red, const uint8_t green, const uint8_t blue) {
    setSingleColor(CRGB(red, green,blue));
}

void DigitalRGB::setSingleColor(const uint32_t rgb) {
    setSingleColor(CRGB(rgb));
}

void DigitalRGB::show() {
    uint8_t global_brightness = FastLED.getBrightness();
    controller->showLeds(global_brightness);
}