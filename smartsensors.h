#pragma once

#include <ESPAsyncWebServer.h>    // ESPAsyncWebServer implementation of LinkedList
#include <Arduino.h>
#define WEATHER_UNDEFINED -1000

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
        virtual void loop() = 0;      // Automatic Periodic Update
        virtual void update() = 0;    // Manual Update
        void setExpirySeconds(const unsigned long expiry);

    protected:
        const String name;
        unsigned long last_update;
        unsigned long expiry_s;

        SmartSensorBase(const String _name, const SensorTypes _type);

    private:
        void constructor();
};

class Weather : public SmartSensorBase {
    
    public:
        Weather(const String _name);
        virtual void loop() = 0;
        virtual void update() = 0;
        const double getTemp() const;
        const double getHumidity() const;

    protected:
        double temp;
        double humid;
        const bool isExpired() const;
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