#include "homekit.h"

bool HomeSpanInit::homespan_init = false;

const bool HomeSpanInit::getInit() {
    return homespan_init;
}

HomeSpanInit::HomeSpanInit() : Loopable(!homespan_init) {     // disable other instances of HomeSpanInit
    homespan_init = true;
}

void HomeSpanInit::loop() {
    homeSpan.poll();
}

HomeSpanAccessory::HomeSpanAccessory(SmartDevice *dev) : SpanAccessory() {
    if (dev == NULL) return;

    init(dev->getName());
    
    // Add more soon
    switch (dev->type) {
        case DeviceType::BrightnessDevice:
        case DeviceType::RGBDevice:
        default:
            new HomeKit_RGB((RGBDevice*) dev);
    }
}

HomeSpanAccessory::HomeSpanAccessory(SmartSensorBase *dev) : SpanAccessory() {
    // TBA
}

void HomeSpanAccessory::init(const String name) {
    // Only init once
    if (!HomeSpanInit::getInit()) {
        new HomeSpanInit();
        homeSpan.begin(Category::Lighting, "ESP32-PC-Lighting", "ESP32-PC-Lighting", "ESP32-Smart");
    }

    new Service::AccessoryInformation();
    new Characteristic::Name(name.c_str());
    new Characteristic::Manufacturer("joshua@josh-wong.net");
    new Characteristic::SerialNumber("JOSH-207");
    new Characteristic::Model("ESP32");
    new Characteristic::FirmwareRevision("2.4");
    new Characteristic::Identify();

    new Service::HAPProtocolInformation();
    new Characteristic::Version("1.2.0");
}

HomeKit_RGB::HomeKit_RGB(RGBDevice *_internaldev) : Service::LightBulb() {
    if (_internaldev == NULL) return;

    internalrgbdevice = _internaldev;
    power = new Characteristic::On();
    H = new Characteristic::Hue();
    S = new Characteristic::Saturation();
    V = new Characteristic::Brightness();
    V->setRange(1, 100, 1); // min 1%, max 100%, in 1% increments
    internal_update();
}

void HomeKit_RGB::loop() {
    if (internalrgbdevice == NULL) return;

    const FrontEnd update_frontend = internalrgbdevice->statusChanged(2);
    if (update_frontend == FrontEnd::None) return;
    if (update_frontend != HOMEKIT_FRONTEND) return;

    internal_update();
}

void HomeKit_RGB::internal_update() {
    if (internalrgbdevice == NULL) {
        Serial.println("HomeKit initialized with nullptr");
        return;
    }

    CHSV fastled_hsv = internalrgbdevice->getHSV();

    power->setVal(internalrgbdevice->getPower());
    H->setVal(fastled_hsv.hue * 360 / 255);
    S->setVal(fastled_hsv.saturation * 100 / 255);
    V->setVal(fastled_hsv.value * 100 / 255);
}

boolean HomeKit_RGB::update() {
    if (internalrgbdevice == NULL) return false;

    // H[0-360]
    // S[0-100]
    // V[0-100]

    float new_h = H->getVal<float>();
    float new_s = S->getVal<float>();
    float new_v = V->getVal<float>();

    if (power->updated()) {
    internalrgbdevice->setPower(power->getNewVal(), HOMEKIT_FRONTEND);
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

    CRGB fastled_rgb;
    const uint8_t uint_h = uint8_t(new_h * 255 / 360 - 1);
    const uint8_t uint_s = uint8_t(new_s * 255 / 100);
    const uint8_t uint_v = uint8_t(new_v * 255 / 100);
    internalrgbdevice->setHSV(CHSV(uint_h, uint_s, uint_v), HOMEKIT_FRONTEND);
}
