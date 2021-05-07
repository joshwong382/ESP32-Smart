#pragma once
#include <ESPAsyncWebServer.h>    // ESPAsyncWebServer implementation of LinkedList
#include "Arduino.h"
#include "frontend.h"
#define PWR_OFF false
#define PWR_ON true

class SmartDevice;
class BrightnessDevice;
class RGBDevice;

extern LinkedList<SmartDevice> smartDevices;

class SmartDevice {

    public:
        SmartDevice(const String _name);

        const bool get_power() const;
        virtual const uint8_t get_brightness_percent() const;
        void set_power(const bool _power, const FrontEnd deviceid);
        const bool flip_power(const FrontEnd deviceid);
        const String get_name() const;
        const FrontEnd status_changed();

    private:
        String name;
        bool power;

    protected:
        FrontEnd status_changed_var;
};


class BrightnessDevice : public SmartDevice {

    public:
        BrightnessDevice(const String _name);

        const uint8_t get_brightness_percent() const;
        void set_brightness_percent(const uint8_t _brightness, const FrontEnd deviceid);

    private:
        uint8_t brightness;

};

class RGBDevice : public BrightnessDevice {

    // Variable brightness not used
    public:
        RGBDevice(const String _name);

        const uint32_t get_rgb() const;
        const String get_rgb_str() const;
        // Device ID that updated
        void set_rgb(const uint8_t _r, const uint8_t _g, const uint8_t _b, const FrontEnd deviceid);
        void set_rgb(const uint32_t rgb, const FrontEnd deviceid);

    private:
        uint8_t r;
        uint8_t g;
        uint8_t b;
        const String colorinttohexstr(uint8_t color) const;

};
