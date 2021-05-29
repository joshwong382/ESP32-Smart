#include "driver.h"

DeviceDriver::DeviceDriver(SmartDevice* _dev) : dev{_dev} {
}

SensorDriver::SensorDriver(SmartSensorBase* _dev) : dev{_dev} {
}