#include "OpenWeatherApi.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TLog.h>

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

    TLog::println(serverPath);

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
        TLog::println("HTTP Response code: ");
        TLog::print(httpResponseCode);
        const String payload = http.getString();
        TLog::println(payload);
    }
    else
    {
        TLog::println("Error code: ");
        TLog::print(httpResponseCode);
    }
    // Free resources
    http.end();
}