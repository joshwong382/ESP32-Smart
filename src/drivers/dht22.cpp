#include "dht22.h"

DHTDriver::DHTDriver(Weather *_dev, const uint8_t _pin, uint8_t dhttype) : SensorDriver{_dev} {
    dht_obj = new DHT_Unified(_pin, dhttype);
    dht_obj -> begin();
}

DHTDriver::~DHTDriver() {
    if (dht_obj != NULL) delete dht_obj;
}

void DHTDriver::loop() {

    auto failure_count = getDev()->getFailureCount();

    // If no failure, refresh in expiry/5
    if (failure_count == 0) {
        if (millis() - getDev()->getLastUpdate() < (getDev()->getExpirySeconds() * 1000 / 5)) return;
        update();
        return;
    }
    
    // last effort update before expiry
    if (getDev()->isExpired()) {
        update();

        // Failed after expiry
        if (failure_count > 0) {
            getDev()->setTemp(WEATHER_UNDEFINED);
            getDev()->setHumidity(WEATHER_UNDEFINED);
        }
        return;
    }

    // Failures but not expired yet, refresh in expiry/15
    if (millis() - getDev()->getLastUpdate() < (getDev()->getExpirySeconds() * 1000 / 15)) return;
    update();
}

void DHTDriver::update() {

    sensors_event_t event;
    dht_obj->temperature().getEvent(&event);

    if (isnan(event.temperature)) {
        getDev()->incrementFailureCount();
        return;
    }

    getDev()->setTemp(event.temperature);

    dht_obj->humidity().getEvent(&event);

    if (isnan(event.relative_humidity)) {
        getDev()->incrementFailureCount();
        return;
    }

    getDev()->setHumidity(event.relative_humidity);

}

Weather* const DHTDriver::getDev() {
    return (Weather*) dev;
}
