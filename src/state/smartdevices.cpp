#include "smartdevices.h"
#include "../StateManager.h"

SmartDevice::SmartDevice(const String _name, const DeviceType _type) : type {_type} {
    constructor(_name);
}

void SmartDevice::updateStatusChanged(const FrontController deviceid) {
    for (int i=0; i<MAX_DRIVERS; i++) {
        status_changed_var[i] = deviceid;
    }
}

void SmartDevice::constructor(const String& _name) {
    name = _name;
    power = PWR_OFF;
    StateManager::addDevice(this);
    updateStatusChanged(FrontController::UpdateAll);
}

SmartDevice::SmartDevice(const String _name) : type {DeviceType::SmartDevice} {
    constructor(_name);
}

const bool SmartDevice::getPower() const {
    return power;
}

const uint8_t SmartDevice::getBrightnessPercent() const {
    return power ? 100 : 0;
}

void SmartDevice::setPower(const bool _power, const FrontController deviceid) {
    updateStatusChanged(deviceid);
    power = _power;
}

const bool SmartDevice::flipPower(const FrontController deviceid) {
    updateStatusChanged(deviceid);
    return power = !power;
}

const String SmartDevice::getName() const {
    return name;
}

const FrontController SmartDevice::statusChanged(const unsigned driver_id) {

    if (driver_id >= MAX_DRIVERS) return FrontController::None;

    FrontController temp_status = status_changed_var[driver_id];
    status_changed_var[driver_id] = FrontController::None;
    return temp_status;
}

// BrightnessDevice

BrightnessDevice::BrightnessDevice(const String _name, const DeviceType _type) : SmartDevice(_name, _type) {}
BrightnessDevice::BrightnessDevice(const String _name) :
            SmartDevice(_name,
            DeviceType::BrightnessDevice) {
    brightness = 100;
}

const uint8_t BrightnessDevice::getBrightnessPercent() const {
    return brightness;
}

void BrightnessDevice::setBrightnessPercent(const uint8_t _brightness, const FrontController deviceid) {
    //Serial.println("brightness request: " + String(_brightness) + " from device " + String(static_cast<int>(deviceid)));
    updateStatusChanged(deviceid);

    if (_brightness == 0)
        setPower(PWR_OFF, FrontController::None);

    else {
        setPower(PWR_ON, FrontController::None);
        if (_brightness > 100)
            brightness = 100;
        else
            brightness = _brightness;
    }
    
}

// RGBDevice

RGBDevice::RGBDevice(const String _name) : BrightnessDevice(_name, DeviceType::RGBDevice) {
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

void RGBDevice::setRGB(const uint32_t rgb, const FrontController deviceid) {
    updateStatusChanged(deviceid);

    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >>  8) & 0xFF;
    uint8_t b = rgb & 0xFF; 
    setRGB(r, g, b, FrontController::None);

}

void RGBDevice::setRGB(const uint8_t _r, const uint8_t _g, const uint8_t _b, const FrontController deviceid) {
    updateStatusChanged(deviceid);

    r = _r;
    g = _g;
    b = _b;
}

const CHSV RGBDevice::getHSV() const {
    const CRGB fastled_rgb = CRGB(getRGB());
    const CHSV fastled_hsv = rgb2hsv_approximate(fastled_rgb);
    return fastled_hsv;
}

void RGBDevice::setHSV(const CHSV& fastled_hsv, const FrontController deviceid) {
    CRGB fastled_rgb;
    hsv2rgb_rainbow(fastled_hsv, fastled_rgb);
    setRGB(fastled_rgb.red, fastled_rgb.green, fastled_rgb.blue, deviceid);
}

const uint8_t RGBDevice::getBrightnessPercent() const {
    CHSV fastled_hsv = getHSV();
    return fastled_hsv.value * 100 / 255;
}

void RGBDevice::setBrightnessPercent(const uint8_t _brightness, const FrontController deviceid) {
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

MusicRGBDevice::MusicRGBDevice(const String _name, const uint8_t trigger_pin) : pin{trigger_pin}, RGBDevice(_name) {
    hue_update_freq = 10;   // How fast the rainbow goes in rainbow mode
    rainbow_hue = 0;
    last_music = false;
    music_status = MusicStatus::IGNORE;
    pinMode(pin, INPUT_PULLUP);
}

const uint8_t MusicRGBDevice::getRainbowHue() {
    return rainbow_hue;
}

void MusicRGBDevice::setRainbowHueDebug(uint8_t hue) {
    rainbow_hue = hue;
}

void MusicRGBDevice::loop() {
    music_status = getMusicStatus();
    EVERY_N_MILLISECONDS(1000/hue_update_freq) {
        rainbow_hue++;
    }
}

const MusicStatus MusicRGBDevice::getMusicStatus() {

    if (!power) {
        return MusicStatus::IGNORE;
    }

    const bool current_music = !digitalRead(pin);

    if (!last_music && !current_music) {
        last_music = current_music;
        return MusicStatus::IGNORE;
    }

    if (last_music && current_music) {
        last_music = current_music;
        return MusicStatus::HOLD;
    }

    if (last_music && !current_music) {
        last_music = current_music;
        return MusicStatus::RELEASE;
    }

    last_music = current_music;

    // Limit how often music can be triggered
    // TBD
    return MusicStatus::INITIATE;
}