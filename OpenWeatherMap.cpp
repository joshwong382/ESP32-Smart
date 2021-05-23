#include "OpenWeatherMap.h"

/* Sample Response
{"coord":{"lon":-1.5,"lat":1.5},"weather":[{"id":814,"main":"Clouds","description":"overcast clouds","icon":"04d"}],
"base":"stations","main":{"temp":227.11,"feels_like":225.6,"temp_min":224.8,"temp_max":221.07,"pressure":921,
"humidity":88,"sea_level":1011,"grnd_level":1012},"visibility":10000,"wind":{"speed":4.96,"deg":214,"gust":8.51},
"clouds":{"all":99},"dt":1661313056,"sys":{"type":2,"id":2004486,"country":"Redacted","sunrise":1513676675,"sunset":1521725508},
"timezone":-12500,"id":6016513,"name":"Redacted","cod":200}
*/

OpenWeatherMap::OpenWeatherMap(const String _name, const unsigned& _city_id, const String& _api_key) : Weather(_name) {
    api_key = _api_key;
    city_id = _city_id;
    failure_count = 0;
}

void OpenWeatherMap::loop() {
    if (failure_count == 0) {
        if (millis() - last_update < (expiry_s * 1000 / 5)) return;
        update();
        return;
    }
    
    // Previous Failures
    if (isExpired()) {
        update();

        // Failed after expiry
        if (failure_count > 0) {
            temp = WEATHER_UNDEFINED;
            humid = WEATHER_UNDEFINED;
        }
    }
}

void OpenWeatherMap::update() {

    if (WiFi.status() != WL_CONNECTED) return;

    failure_count++;
    String json_response;
    const bool success = httpRequest(json_response);
    last_update = millis();

    // Parse JSON
    DynamicJsonDocument json(2048);
    DeserializationError error = deserializeJson(json, json_response.c_str());
    if (error) {
        Serial.println("Get OpenWeatherMap data failure count: " + String(failure_count));
        return;
    }

    JsonVariant temp_json = json["main"]["temp"];
    JsonVariant humid_json = json["main"]["humidity"];

    double _temp;
    double _humid;

    // Only fail if no temperature
    if (jsonToPositiveDouble(temp_json, _temp)) {
        temp = _temp + ABSOLUTE_ZERO;
        failure_count = 0;
    }

    if (jsonToPositiveDouble(humid_json, _humid)) {
        humid = _humid;
    }
}

void OpenWeatherMap::setAPIKey(const String& _api_key) {
    api_key = _api_key;
}

void OpenWeatherMap::setCityID(const unsigned& _city_id) {
    city_id = _city_id;
}

const bool OpenWeatherMap::jsonToPositiveDouble(const JsonVariant& json_variant, double& value) const {
    if (json_variant.is<double>()) {
        double _double = json_variant.as<double>();

        // do not allow negative numbers
        if (_double > 0) {
            value = _double;
            return true;
        }
    }
    return false;
}

const String OpenWeatherMap::createURL(const unsigned& city_id, const String& api_key) const {
    return String(BASE_URL) + "?id=" + String(city_id) + "&appid=" + api_key;
}

const bool OpenWeatherMap::httpRequest(String& response) const {

    if (WiFi.status() != WL_CONNECTED) return "";

    const String url = createURL(city_id, api_key);
    HTTPClient http;

    // request
    http.begin(url.c_str());
    http.setTimeout(2000);
    const unsigned response_code = http.GET();

    // response
    if (response_code > 0) response = http.getString();
    else response = "";

    http.end();

    if (response_code != 200) return false;

    // HTTP OK
    return true;
}