#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "smartsensors.h"
#define ABSOLUTE_ZERO -273.15
#define BASE_URL "http://api.openweathermap.org/data/2.5/weather"

class OpenWeatherMap : public Weather {

    public:
        OpenWeatherMap(const String name, const unsigned& _city_id, const String& _api_key);
        void loop();
        void update();                              // Failure only occurs when temperature is missing
        void setAPIKey(const String& _api_key);
        void setCityID(const unsigned& _city_id);

    private:
        String api_key;
        unsigned city_id;
        unsigned failure_count;
        const bool jsonToPositiveDouble(const JsonVariant& json_variant, double& value) const;
        const String createURL(const unsigned& city_id, const String& api_key) const;
        const bool httpRequest(String& response) const;
};