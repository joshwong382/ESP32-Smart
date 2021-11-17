#pragma once
#include <DHT.h>
#include <DHT_U.h>
#include "../driver.h"

class DHTDriver : public SensorDriver {

    public:
        DHTDriver(Weather *_dev, const uint8_t _pin, uint8_t dhttype);
        ~DHTDriver();
        void loop();
        void update();

    private:
        DHT_Unified* dht_obj;
        Weather* const getDev();
};
