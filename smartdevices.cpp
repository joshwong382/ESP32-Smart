#include "smartdevice.h"

LinkedList<SmartDevice*> smartDevices([](const SmartDevice* dev) {
    delete dev;
});

SmartDevice::SmartDevice(const String _name, const Backend _type) : type {_type} {
    constructor(_name);
}

void SmartDevice::constructor(const String& _name) {
    name = _name;
    power = PWR_OFF;
    status_changed_var = FrontEnd::UpdateAll;
}

SmartDevice::SmartDevice(const String _name) : type {Backend::SmartDevice} {
    constructor(_name);
}

const bool SmartDevice::getPower() const {
    return power;
}

const uint8_t SmartDevice::getBrightnessPercent() const {
    return power ? 100 : 0;
}

void SmartDevice::setPower(const bool _power, const FrontEnd deviceid) {
    status_changed_var = deviceid;
    power = _power;
}

const bool SmartDevice::flipPower(const FrontEnd deviceid) {
    status_changed_var = deviceid;
    return power = !power;
}

const String SmartDevice::getName() const {
    return name;
}

const FrontEnd SmartDevice::statusChanged() {
    FrontEnd temp_status = status_changed_var;
    status_changed_var = FrontEnd::None;
    return temp_status;
}

// BrightnessDevice

BrightnessDevice::BrightnessDevice(const String _name, const Backend _type) : SmartDevice(_name, _type) {}
BrightnessDevice::BrightnessDevice(const String _name) : SmartDevice(_name, Backend::BrightnessDevice) {
    brightness = 100;
}

const uint8_t BrightnessDevice::getBrightnessPercent() const {
    return brightness;
}

void BrightnessDevice::setBrightnessPercent(const uint8_t _brightness, const FrontEnd deviceid) {
    Serial.println("brightness request: " + String(_brightness) + " from device " + String(static_cast<int>(deviceid)));
    status_changed_var = deviceid;

    if (_brightness == 0)
        setPower(PWR_OFF, FrontEnd::None);

    else {
        setPower(PWR_ON, FrontEnd::None);
        if (_brightness > 100)
            brightness = 100;
        else
            brightness = _brightness;
    }
    
}

// RGBDevice

RGBDevice::RGBDevice(const String _name) : BrightnessDevice(_name, Backend::RGBDevice) {
    r = 255;
    b = 255;
    g = 255;
}

const uint32_t RGBDevice::getRGB() const {
    return (r << 16) + (g << 8) + b;
}

const String RGBDevice::getRGBStr() const {
    return colorAsHEXStr(r) + colorAsHEXStr(g) + colorAsHEXStr(b);
}

void RGBDevice::setRGB(const uint32_t rgb, const FrontEnd deviceid) {

    status_changed_var = deviceid;

    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >>  8) & 0xFF;
    uint8_t b = rgb & 0xFF; 
    setRGB(r, g, b, FrontEnd::None);

}

void RGBDevice::setRGB(const uint8_t _r, const uint8_t _g, const uint8_t _b, const FrontEnd deviceid) {

    status_changed_var = deviceid;

    r = _r;
    g = _g;
    b = _b;
}

const CHSV RGBDevice::getHSV() const {
    const CRGB fastled_rgb = CRGB(getRGB());
    const CHSV fastled_hsv = rgb2hsv_approximate(fastled_rgb);
    return fastled_hsv;
}

void RGBDevice::setHSV(const CHSV& fastled_hsv, const FrontEnd deviceid) {
    CRGB fastled_rgb;
    hsv2rgb_rainbow(fastled_hsv, fastled_rgb);
    setRGB(fastled_rgb.red, fastled_rgb.green, fastled_rgb.blue, deviceid);
}

const uint8_t RGBDevice::getBrightnessPercent() const {
    CHSV fastled_hsv = getHSV();
    return fastled_hsv.value * 100 / 255;
}

void RGBDevice::setBrightnessPercent(const uint8_t _brightness, const FrontEnd deviceid) {
    CHSV fastled_hsv = getHSV();
    fastled_hsv.value = _brightness * 255 / 100;
    setHSV(fastled_hsv, deviceid);
}

// Private
const String RGBDevice::colorAsHEXStr(const uint8_t color) const {
  const String a = String(color, HEX);
  if (a.length() == 0) {
    return "00";
  }

  if (a.length() == 1) {
    return "0" + a;
  }

  if (a.length() > 2) {
    return "00";
  }
  return a;
}