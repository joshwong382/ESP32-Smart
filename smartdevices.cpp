#pragma once
#include "smartdevice.h"

SmartDevice::SmartDevice(const String _name) {
    name = _name;
    power = PWR_OFF;
    status_changed_var = FrontEnd::UpdateAll;
}

const bool SmartDevice::get_power() const {
    return power;
}

const uint8_t SmartDevice::get_brightness_percent() const {
    return power ? 100 : 0;
}

void SmartDevice::set_power(const bool _power, const FrontEnd deviceid) {
    status_changed_var = deviceid;
    power = _power;
}

const bool SmartDevice::flip_power(const FrontEnd deviceid) {
    status_changed_var = deviceid;
    return power = !power;
}

const String SmartDevice::get_name() const {
    return name;
}

const FrontEnd SmartDevice::status_changed() {
    FrontEnd temp_status = status_changed_var;
    status_changed_var = FrontEnd::None;
    return temp_status;
}

// BrightnessDevice

BrightnessDevice::BrightnessDevice(const String _name):SmartDevice(_name) {
    brightness = 100;
}

const uint8_t BrightnessDevice::get_brightness_percent() const {
    return brightness;
}

void BrightnessDevice::set_brightness_percent(const uint8_t _brightness, const FrontEnd deviceid) {
    Serial.println("brightness request: " + String(_brightness) + " from device " + String(static_cast<int>(deviceid)));
    status_changed_var = deviceid;

    if (_brightness == 0)
        set_power(PWR_OFF, FrontEnd::None);

    else {
        set_power(PWR_ON, FrontEnd::None);
        if (_brightness > 100)
            brightness = 100;
        else
            brightness = _brightness;
    }
    
}

// RGBDevice

RGBDevice::RGBDevice(const String _name):BrightnessDevice(_name) {
    r = 255;
    b = 255;
    g = 255;
}

const uint32_t RGBDevice::get_rgb() const {
    return (r << 16) + (g << 8) + b;
}

const String RGBDevice::get_rgb_str() const {
    return colorinttohexstr(r) + colorinttohexstr(g) + colorinttohexstr(b);
}

void RGBDevice::set_rgb(const uint32_t rgb, const FrontEnd deviceid) {

    status_changed_var = deviceid;

    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >>  8) & 0xFF;
    uint8_t b = rgb & 0xFF; 
    set_rgb(r, g, b, FrontEnd::None);

}

void RGBDevice::set_rgb(const uint8_t _r, const uint8_t _g, const uint8_t _b, const FrontEnd deviceid) {

    status_changed_var = deviceid;

    r = _r;
    g = _g;
    b = _b;
}

// Private
const String RGBDevice::colorinttohexstr(const uint8_t color) const {
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