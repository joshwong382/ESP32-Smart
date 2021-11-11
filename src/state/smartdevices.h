#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "../controller.h"
#include "../loopable.h"
#define PWR_OFF false
#define PWR_ON true
#define MAX_DRIVERS 3

class SmartDevice;
class BrightnessDevice;
class RGBDevice;

enum class DeviceType {
    SmartDevice,        // aka switch
    BrightnessDevice,   // switch + brightness
    RGBDevice           // switch + brightness + RGB
};

enum class MusicStatus {
    IGNORE,
    INITIATE,
    RELEASE,
    HOLD
};

class SmartDevice {

    protected:
        String name;
        bool power;
        Controller status_changed_var[MAX_DRIVERS];

        SmartDevice(const String _name, const DeviceType _type);
        SmartDevice();
        void updateStatusChanged(const Controller deviceid);

    private:
        void constructor(const String& _name);

    public:
        const DeviceType type;

        SmartDevice(const String _name);
        const bool getPower() const;
        virtual const uint8_t getBrightnessPercent() const;
        void setPower(const bool _power, const Controller deviceid);      // which Controller triggered the change
        const bool flipPower(const Controller deviceid);
        const String getName() const;
        const Controller statusChanged(const unsigned driver_id);
        
};

class BrightnessDevice : public SmartDevice {

    protected:
        BrightnessDevice(const String _name, const DeviceType _type);
    
    public:
        BrightnessDevice(const String _name);

        virtual const uint8_t getBrightnessPercent() const;
        virtual void setBrightnessPercent(const uint8_t _brightness, const Controller deviceid);

    private:
        uint8_t brightness;

};

class RGBDevice : public BrightnessDevice {

    // Variable brightness not used
    public:
        RGBDevice(const String _name);

        const uint32_t getRGB() const;
        const String getRGBStr() const;
        void setRGB(const uint8_t _r, const uint8_t _g, const uint8_t _b, const Controller deviceid);
        void setRGB(const uint32_t rgb, const Controller deviceid);
        const CHSV getHSV() const;
        void setHSV(const CHSV& fastled_hsv, const Controller deviceid);
        const uint8_t getBrightnessPercent() const;
        void setBrightnessPercent(const uint8_t _brightness, const Controller deviceid);

    private:
        uint8_t r;
        uint8_t g;
        uint8_t b;
        const String colorAsHEXStr(uint8_t color) const;

};

class MusicRGBDevice : public RGBDevice, Loopable {

    public:
        MusicRGBDevice(const String _name, const uint8_t trigger_pin);
        const uint8_t getRainbowHue();
        void setRainbowHueDebug(uint8_t hue);
        void loop();
        MusicStatus music_status;

    private:
        const MusicStatus getMusicStatus();
        bool last_music;                // Last pin state of MusicTrig
        const uint8_t pin;              // Pin number
        uint8_t rainbow_hue;
        unsigned hue_update_freq;
};