#include "OpenWeatherApi.h"
#include "HttpClientAux.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <optional>

#include "functional_aux.h"

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
namespace OpenWeather
{
constexpr char TAG[] = "[OpenWeatherApi]";
constexpr char BASE_API_URL[] = "https://api.openweathermap.org/data/2.5/weather";

std::optional<Weather_t> weather;

auto DecomposeJson(JsonDocument&& aDoc) -> Weather_t
{
    return Weather_t {
        aDoc["main"]["temp"].as<double>()
    };
}

auto ValidateJsonResponse(const String& aData) -> Functional::Optional<JsonDocument>
{
    // Allocate the JSON document
    JsonDocument doc;

    // Deserialize the JSON document
    // Test if parsing succeeds.
    if (auto error = deserializeJson(doc, aData); error)
    {
        log_w("%s JSON deserialition failed. Error code: %s", TAG, error.c_str());
        return std::nullopt;
    }

    // Test if json is applicable
    if(!doc["cod"].is<int>())
        return std::nullopt;

    constexpr int respondOK{200};
    constexpr int respondGoodKeyInvalidData{400};

    const int code = doc["cod"].as<int>();

    if (!(respondOK == code || respondGoodKeyInvalidData == code))
        return std::nullopt;

    return doc;
}

auto BuildApiUrl(const String &aApiKey, const String &aLat, const String &aLon)-> String
{
    constexpr char units[]{"metric"};
    return String(BASE_API_URL)
                + "?lat=" + aLat
                + "&lon=" + aLon
                + "&appid=" + aApiKey
                + "&units=" + units;
}

auto FetchData(const String &aApiKey, const String &aLat, const String &aLon) -> bool
{
    const String requestUrl = BuildApiUrl(aApiKey, aLat, aLon);
    log_d("%s %s", TAG, requestUrl.c_str());

    if (auto res = Functional::Optional(SendGetRequest(requestUrl))
                                        .and_then(ValidateJsonResponse)
                                        .transform(DecomposeJson)
                                        .into_optional()
        ; res)
    {
        weather = res;
        // Print values.
        log_i("%s Acquired temperature: [ %4.1f ]", TAG, weather.value().temp);

        return true;
    }

    return false;
}
}