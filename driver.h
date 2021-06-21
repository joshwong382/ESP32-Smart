#pragma once
#include "smartdevices.h"
#include "smartsensors.h"

class DeviceDriver {

    protected:
        SmartDevice* const dev;

    public:
        DeviceDriver(SmartDevice* _dev);
        virtual void loop() = 0;
};

class MusicDeviceDriver : public DeviceDriver {

    protected:
        MusicRGBDevice* const musicdev;

    public:
        MusicDeviceDriver(RGBDevice* _dev, MusicRGBDevice* _musicdev);
        virtual void loop() = 0;
};

class SensorDriver {

    protected:
        SmartSensorBase* const dev;

    public:
        SensorDriver(SmartSensorBase* _dev);
        virtual void loop() = 0;
        virtual void update() = 0;
};