#include "IpGeolocationApi.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

Coordinates Coordinates_;

/*
api description: https://ipgeolocation.io/ip-location-api.html#documentation-overview
*/

auto GetLocationCoordinates(const String &aApiKey) -> void
{ // [ ]FIXME:rename
    const String serverName = "https://api.ipgeolocation.io/ipgeo";

    const String serverPath = serverName + "?apiKey=" + aApiKey;

    log_d("%s", serverPath.c_str());

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
            log_w("JSON deserializition is failed. Error code: %s", error.f_str());
            return;
        }

        // Fetch values.
        //
        // Most of the time, you can rely on the implicit casts.
        // In other case, you can do doc["time"].as<long>();
        Coordinates_.latitude = doc["latitude"];
        Coordinates_.longitude = doc["longitude"];

        // Print values.
        log_i("Acquired coordinates: [%3.3f, %3.3f]", Coordinates_.latitude, Coordinates_.longitude);
    }
    else
    {
        log_w("Error code: %d", httpResponseCode);
    }
    // Free resources
    http.end();
}