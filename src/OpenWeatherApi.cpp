#include "OpenWeatherApi.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

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

auto GetForecast(const String &aApiKey, const String &aLat, const String &aLon) -> void // [ ]FIXME:rename
{
    const String serverName = "https://api.openweathermap.org/data/2.5/weather";
    const String units{"metric"};
    const String serverPath = serverName + "?lat=" + aLat + "&lon=" + aLon + "&appid=" + aApiKey + "&units=" + units;

    log_d("%s", serverPath);

    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // [ ]FIXME:remove
    // If you need Node-RED/server authentication, insert user and password below
    // http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        // [ ]TODO: handle response codes and JSONs 200 400 401 404
        log_d("HTTP Response code: %d", httpResponseCode);
        const String payload = http.getString();
        // log_d("%s", payload.c_str());

        // Allocate the JSON document
        JsonDocument doc;
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds.
        if (error)
        {
            log_w("JSON deserialition failed. Error code: %s", error.c_str());
            return;
        }

        // Fetch values.
        //
        // Most of the time, you can rely on the implicit casts.
        // In other case, you can do doc["time"].as<long>();
        double temp = doc["main"]["temp"];
        // Coordinates_.longitude = doc["longitude"];

        // // Print values.
        log_d("Acquired temperature: [ %4.1f ]", temp);
    }
    else
    {
        log_d("Error code: %d", httpResponseCode);
    }
    // Free resources
    http.end();
}