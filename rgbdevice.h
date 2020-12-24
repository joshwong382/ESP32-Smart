#include "Arduino.h"
#define PWR_OFF false
#define PWR_ON true

class Device {

    public:
    Device(String _name);

    bool get_power();
    void set_power(bool _power);
    bool flip_power();
    String get_name();

    private:
    String name;
    bool power;
};


class BrightnessDevice : public Device {

    public:
    BrightnessDevice(String _name);

    uint8_t get_brightness_percent();
    void set_brightness_percent(uint8_t _brightness);

    private:
    uint8_t brightness;

};

class RGBDevice : public BrightnessDevice {

    // Variable brightness not used
    public:
    RGBDevice(String _name);

    uint32_t get_rgb();
    String get_rgb_str();
    void set_rgb(uint8_t _r, uint8_t _g, uint8_t _b);
    void set_rgb(uint32_t rgb);

    private:

    uint8_t r;
    uint8_t g;
    uint8_t b;
    String colorinttohexstr(uint8_t color);

};