#include "OpenWeatherApi.h"
#include "HttpClientAux.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
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
    constexpr char units[]{"metric"};
    return String("https://api.openweathermap.org/data/2.5/weather")
                + "?lat=" + aLat
                + "&lon=" + aLon
                + "&appid=" + aApiKey
                + "&units=" + units;
}

auto GetForecast(const String &aApiKey, const String &aLat, const String &aLon) -> bool // [ ]FIXME:rename
{
    const String requestUrl = GetApiUrl(aApiKey, aLat, aLon);
    log_d("%s", requestUrl.c_str());

    auto respond = SendGetRequest(requestUrl);
    if(!respond)
        return false;

    const auto res = ParseJsonOwtr(respond.value());

    if (!res)
        return false;

    weather = res.value();
    // Print values.
    log_i("Acquired temperature: [ %4.1f ]", weather.temp);

    return true;
}