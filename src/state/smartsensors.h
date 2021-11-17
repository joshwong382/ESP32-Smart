#pragma once

#include <ESPAsyncWebServer.h>    // ESPAsyncWebServer implementation of LinkedList
#include <Arduino.h>
#include "../loopable.h"
#define WEATHER_UNDEFINED -1000
#define ABSOLUTE_ZERO -273.15

class SmartSensorBase;
class Weather;

enum class SensorType {
    SmartSensorBase,
    Weather
};

class SmartSensorBase {

    public:
        const SensorType type;
        
        SmartSensorBase(const String _name);
        const String getName() const;
        const unsigned getFailureCount() const;
        const unsigned long getLastUpdate() const;
        const unsigned long getExpirySeconds() const;
        void incrementFailureCount();
        void resetLastUpdate();
        void setExpirySeconds(const unsigned long expiry);

    protected:
        const String name;
        unsigned long last_update;
        unsigned long expiry_s;
        unsigned failure_count;

        SmartSensorBase(const String _name, const SensorType _type);

    private:
        void constructor();
};

class Weather : public SmartSensorBase {

    public:
        Weather(const String _name);

        // Get current data
        const double getTemp() const;
        const double getHumidity() const;

        // Set current data
        void setTemp(const double _temp);           // failure_count resets if setTemp succeeds. It ignores setHumidity.
        void setHumidity(const double _humid);
        const bool isExpired() const;

    protected:
        double temp;
        double humid;
};