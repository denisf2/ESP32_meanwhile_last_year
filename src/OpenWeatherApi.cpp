#include "OpenWeatherApi.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <optional>

/*
api description: https://openweathermap.org/current
How to make an API call
```
const String WEATHER_API = "https://api.openweathermap.org/data/2.5/weather";
```

not supported anymore api: https://openweathermap.org/api/geocoding-api
Direct geocoding converts the specified name of a location or
zip/post code into the exact geographical coordinates;
```
const String GEO_DIRECT_API = "https://api.openweathermap.org/geo/1.0/direct";
```
Reverse geocoding converts the geographical coordinates into
the names of the nearby locations.
```
const String GEO_REVERSE_API = "https://api.openweathermap.org/geo/1.0/reverse";
```
*/

Weather_t weather;

auto ParseJsonOwtr(const String& aData) -> std::optional<Weather_t>
{
    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, aData);

    // Test if parsing succeeds.
    if (error)
    {
        log_w("JSON deserialition failed. Error code: %s", error.c_str());
        return std::nullopt;
    }

    if(!doc["cod"].is<int>())
        return std::nullopt;

    constexpr int respondOK{200};
    constexpr int respondGoodKeyInvalidData{400};

    const int code = doc["cod"].as<int>();

    if (!(respondOK == code || respondGoodKeyInvalidData == code))
        return std::nullopt;

    // Fetch values.
    Weather_t w;
    w.temp = doc["main"]["temp"];

    return std::make_optional(std::move(w));
}

auto GetApiUrl(const String &aApiKey, const String &aLat, const String &aLon)-> String
{
    const String units{"metric"};
    return String("https://api.openweathermap.org/data/2.5/weather")
                + "?lat=" + aLat
                + "&lon=" + aLon
                + "&appid=" + aApiKey
                + "&units=" + units;
}

auto GetForecast(const String &aApiKey, const String &aLat, const String &aLon) -> bool // [ ]FIXME:rename
{
    const String serverPath = GetApiUrl(aLat, aLon, aApiKey);
    log_d("%s", serverPath.c_str());

    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // If you need server authentication, insert user and password below
    // http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode <= 0)
    {
        log_d("Error code: %d", httpResponseCode);
        http.end();
        return false;
    }

    // [ ]TODO: handle response codes and JSONs 200 400 401 404
    log_d("HTTP Response code: %d", httpResponseCode);
    const String payload = http.getString();
    // log_d("%s", payload.c_str());

    auto res = ParseJsonOwtr(payload);

    // Free resources
    http.end();

    if(!res)
        return false;

    weather = res.value();

    // Print values.
    log_d("Acquired temperature: [ %4.1f ]", weather.temp);

    return true;
}