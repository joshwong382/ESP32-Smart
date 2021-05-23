#pragma once
#include <ESPAsyncWebServer.h>    // ESPAsyncWebServer implementation of LinkedList
#include <Arduino.h>
#include <FastLED.h>
#include "frontend.h"
#define PWR_OFF false
#define PWR_ON true

class SmartDevice;
class BrightnessDevice;
class RGBDevice;

extern LinkedList<SmartDevice*> smartDevices;

enum class Backend {
    SmartDevice,        // aka switch
    BrightnessDevice,   // switch + brightness
    RGBDevice           // switch + brightness + RGB
};

class SmartDevice {

    protected:
        String name;
        bool power;
        FrontEnd status_changed_var;

        SmartDevice(const String _name, const Backend _type);

    private:
        void constructor(const String& _name);

    public:
        const Backend type;

        SmartDevice(const String _name);
        const bool getPower() const;
        virtual const uint8_t getBrightnessPercent() const;
        void setPower(const bool _power, const FrontEnd deviceid);
        const bool flipPower(const FrontEnd deviceid);
        const String getName() const;
        const FrontEnd statusChanged();
        
};


class BrightnessDevice : public SmartDevice {

    protected:
        BrightnessDevice(const String _name, const Backend _type);
    
    public:
        BrightnessDevice(const String _name);

        virtual const uint8_t getBrightnessPercent() const;
        virtual void setBrightnessPercent(const uint8_t _brightness, const FrontEnd deviceid);

    private:
        uint8_t brightness;

};

class RGBDevice : public BrightnessDevice {

    // Variable brightness not used
    public:
        RGBDevice(const String _name);

        const uint32_t getRGB() const;
        const String getRGBStr() const;
        void setRGB(const uint8_t _r, const uint8_t _g, const uint8_t _b, const FrontEnd deviceid);
        void setRGB(const uint32_t rgb, const FrontEnd deviceid);
        const CHSV getHSV() const;
        void setHSV(const CHSV& fastled_hsv, const FrontEnd deviceid);
        const uint8_t getBrightnessPercent() const;
        void setBrightnessPercent(const uint8_t _brightness, const FrontEnd deviceid);

    private:
        uint8_t r;
        uint8_t g;
        uint8_t b;
        const String colorAsHEXStr(uint8_t color) const;

};
