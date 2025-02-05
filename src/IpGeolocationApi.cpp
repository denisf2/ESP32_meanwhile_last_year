#include "IpGeolocationApi.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TLog.h>

Coordinates Coordinates_;

/*
api description: https://ipgeolocation.io/ip-location-api.html#documentation-overview
*/

auto GetLocationCoordinates(const String &aApiKey) -> void
{ // [ ]FIXME:rename
    const String serverName = "https://api.ipgeolocation.io/ipgeo";

    const String serverPath = serverName + "?apiKey=" + aApiKey;

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
        // [ ]TODO: handle response codes and JSONs 200 401 404
        TLog::println("HTTP Response code: ");
        TLog::print(httpResponseCode);
        const String payload = http.getString();
        // TLog::println(payload);

        // Allocate the JSON document
        JsonDocument doc;
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, payload);

        // Test if parsing succeeds.
        if (error)
        {
            TLog::println(F("deserializeJson() failed: "));
            TLog::print(error.f_str());
            return;
        }

        // Fetch values.
        //
        // Most of the time, you can rely on the implicit casts.
        // In other case, you can do doc["time"].as<long>();
        Coordinates_.latitude = doc["latitude"];
        Coordinates_.longitude = doc["longitude"];

        // Print values.
        TLog::println("Acquired coordinates: [");
        TLog::print(Coordinates_.latitude, 6);
        TLog::print(", ");
        TLog::print(Coordinates_.longitude, 6);
        TLog::print("]");
    }
    else
    {
        TLog::println("Error code: ");
        TLog::print(httpResponseCode);
    }
    // Free resources
    http.end();
}