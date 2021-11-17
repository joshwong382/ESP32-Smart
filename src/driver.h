#pragma once
#include "state/smartdevices.h"
#include "state/smartsensors.h"
#include "loopable.h"

// Drivers should be stateless
// Drivers should implement getDev() since dev comes from Superclasses here
// Poll state in loop() from getDev()

class DeviceDriver : public Loopable {

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

class SensorDriver : public Loopable {

    protected:
        SmartSensorBase* const dev;

    public:
        SensorDriver(SmartSensorBase* _dev);
        virtual void loop() = 0;                // Should call update() in interval defined in the smartsensor state
        virtual void update() = 0;              // Update data
};

template<typename T>
class DriverMonitorControllerUpdatable {

    private:
        T value;
        T default_value;

    public:
        DriverMonitorControllerUpdatable(T& t);
        const T getReset();
        void set(T& t);

};

template<typename T>
class DriverMonitorControllerUpdatableDelegate {



};