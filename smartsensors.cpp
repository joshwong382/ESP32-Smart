#include "smartsensors.h"
#include <WiFi.h>

LinkedList<SmartSensorBase*> smartSensors([](const SmartSensorBase* dev) {
    delete dev;
});

SmartSensorBase::SmartSensorBase(const String _name) : name {_name}, type {SensorTypes::SmartSensorBase} {
    constructor();
}

SmartSensorBase::SmartSensorBase(const String _name, const SensorTypes _type) : name {_name}, type {_type} {
    constructor();
}

void SmartSensorBase::constructor() {
    last_update = 0;
    expiry_s = 300;
}

const String SmartSensorBase::getName() const {
    return name;
}

void SmartSensorBase::setExpirySeconds(const unsigned long expiry) {
    expiry_s = expiry;
} 

Weather::Weather(const String _name) : SmartSensorBase(_name, SensorTypes::Weather) {
    temp = WEATHER_UNDEFINED;
    humid = WEATHER_UNDEFINED;
}

const double Weather::getTemp() const {
    return temp;
}

const double Weather::getHumidity() const {
    return humid;
}

const bool Weather::isExpired() const {
    return millis() - last_update > expiry_s * 1000;
}
