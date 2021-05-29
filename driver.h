#include "smartdevices.h"
#include "smartsensors.h"

class DeviceDriver {

    protected:
        const SmartDevice* dev;

    public:
        DeviceDriver(SmartDevice* _dev);
        virtual void loop() = 0;
};

class SensorDriver {

    protected:
        const SmartSensorBase* dev;

    public:
        SensorDriver(SmartSensorBase* _dev);
        virtual void loop() = 0;
        virtual void update() = 0;
};