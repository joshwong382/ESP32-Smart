#include "Arduino.h"
#define PWR_OFF false
#define PWR_ON true

class Device {

    public:
    Device(String _name);

    enum Devices {
      None = 0,
      HomeKit = 1,
      HTTP_API = 2,
      UpdateAll = 3
    };

    bool get_power();
    void set_power(bool _power, Devices deviceid);
    bool flip_power(Devices deviceid);
    String get_name();
    Devices status_changed();

    private:
    String name;
    bool power;

    protected:
    Devices status_changed_var;
};


class BrightnessDevice : public Device {

    public:
    BrightnessDevice(String _name);

    uint8_t get_brightness_percent();
    void set_brightness_percent(uint8_t _brightness, Device::Devices deviceid);

    private:
    uint8_t brightness;

};

class RGBDevice : public BrightnessDevice {

    // Variable brightness not used
    public:
    RGBDevice(String _name);

    uint32_t get_rgb();
    String get_rgb_str();
    // Device ID that updated
    void set_rgb(uint8_t _r, uint8_t _g, uint8_t _b, Device::Devices deviceid);
    void set_rgb(uint32_t rgb, Device::Devices deviceid);

    private:

    uint8_t r;
    uint8_t g;
    uint8_t b;
    String colorinttohexstr(uint8_t color);

};