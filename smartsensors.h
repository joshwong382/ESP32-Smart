#pragma once

#include <ESPAsyncWebServer.h>    // ESPAsyncWebServer implementation of LinkedList
#include <Arduino.h>
#define WEATHER_UNDEFINED -1000
#define ABSOLUTE_ZERO -273.15

class SmartSensorBase;
class Weather;

extern LinkedList<SmartSensorBase*> smartSensors;

enum class SensorTypes {
    SmartSensorBase,
    Weather
};

class SmartSensorBase {

    public:
        const SensorTypes type;
        
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

        SmartSensorBase(const String _name, const SensorTypes _type);

    private:
        void constructor();
};

class Weather : public SmartSensorBase {

    public:
        Weather(const String _name);
        const double getTemp() const;
        const double getHumidity() const;
        void setTemp(const double _temp);           // failure_count resets if setTemp succeeds. It ignores setHumidity.
        void setHumidity(const double _humid);
        const bool isExpired() const;

    protected:
        double temp;
        double humid;
};

// Move to own file
/*
class DHT22 : public Weather {

    public:
        DHT22();
        void loop();
        void update();
};
*/