#include "rgbdevice.h"

Device::Device(String _name) {
    name = _name;
    power = PWR_OFF;
    status_changed_var = true;
}

bool Device::get_power() {
    return power;
}

void Device::set_power(bool _power) {
    status_changed_var = true;
    power = _power;
}

bool Device::flip_power() {
    status_changed_var = true;
    return power = !power;
}

String Device::get_name() {
    return name;
}

bool Device::status_changed() {
    bool temp_status = status_changed_var;
    status_changed_var = false;
    return temp_status;
}

// BrightnessDevice
// Public

BrightnessDevice::BrightnessDevice(String _name):Device(_name) {
    brightness = 100;
}

uint8_t BrightnessDevice::get_brightness_percent() {
    return brightness;
}

void BrightnessDevice::set_brightness_percent(uint8_t _brightness) {
    status_changed_var = true;

    if (_brightness == 0)
        set_power(PWR_OFF);

    else {
        set_power(PWR_ON);
        if (_brightness > 100)
            brightness = 100;
        else
            brightness = _brightness;
    }
    
}


// RGBDevice
// Public

RGBDevice::RGBDevice(String _name):BrightnessDevice(_name) {
    r = 255;
    b = 255;
    g = 255;
}

uint32_t RGBDevice::get_rgb() {
    return (r << 16) + (g << 8) + b;
}

String RGBDevice::get_rgb_str() {
    return colorinttohexstr(r) + colorinttohexstr(g) + colorinttohexstr(b);
}

void RGBDevice::set_rgb(uint32_t rgb) {

    status_changed_var = true;

    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >>  8) & 0xFF;
    uint8_t b = rgb & 0xFF; 
    set_rgb(r, g, b);

}

void RGBDevice::set_rgb(uint8_t _r, uint8_t _g, uint8_t _b) {

    status_changed_var = true;

    r = _r;
    g = _g;
    b = _b;
}


// Private

String RGBDevice::colorinttohexstr(uint8_t color) {
  String a = String(color, HEX);
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