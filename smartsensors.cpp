#include "smartsensors.h"
#include <WiFi.h>

LinkedList<SmartSensorBase*> smartSensors([](const SmartSensorBase* dev) {
    if (dev != NULL) {
        delete dev;
    }
});

SmartSensorBase::SmartSensorBase(const String _name) :
            name {_name},
            type {SensorTypes::SmartSensorBase} {
    constructor();
}

SmartSensorBase::SmartSensorBase(const String _name, const SensorTypes _type) :
            name {_name},
            type {_type} {
    constructor();
}

void SmartSensorBase::constructor() {
    last_update = 0;
    expiry_s = 300;
    failure_count = 0;
}

const String SmartSensorBase::getName() const {
    return name;
}

const unsigned SmartSensorBase::getFailureCount() const {
    return failure_count;
}

const unsigned long SmartSensorBase::getLastUpdate() const {
    return last_update;
}

const unsigned long SmartSensorBase::getExpirySeconds() const {
    return expiry_s;
}

void SmartSensorBase::incrementFailureCount() {
    failure_count++;
}

void SmartSensorBase::resetLastUpdate() {
    last_update = millis();
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

void Weather::setTemp(const double _temp) {

    // if WEATHER_UNDEFINED, it will be below absolute zero
    if (_temp < ABSOLUTE_ZERO && _temp != WEATHER_UNDEFINED) {
        return;
    }

    temp = _temp;
    failure_count = 0;
}

void Weather::setHumidity(const double _humid) {
    if (_humid <= 0 && _humid != WEATHER_UNDEFINED) return;
    humid = _humid;
}

const bool Weather::isExpired() const {
    return millis() - last_update > expiry_s * 1000;
}
