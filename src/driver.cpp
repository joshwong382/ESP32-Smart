#include <Arduino.h>
#include "driver.h"

DeviceDriver::DeviceDriver(SmartDevice* _dev) : dev{_dev} {
}

MusicDeviceDriver::MusicDeviceDriver(RGBDevice* _dev, MusicRGBDevice* _musicdev) :
            musicdev{_musicdev},
            DeviceDriver(_dev) {
}

SensorDriver::SensorDriver(SmartSensorBase* _dev) : dev{_dev} {
}