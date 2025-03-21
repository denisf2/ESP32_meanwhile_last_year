#include "IpGeolocationApi.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <optional>

Coordinates Coordinates_;

/*
api description: https://ipgeolocation.io/ip-location-api.html#documentation-overview
*/

auto ParseJson(const String &aData) -> std::optional<Coordinates>
{
    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, aData);
    // Test if parsing succeeds.
    if (error)
    {
        log_w("JSON deserializition is failed. Error code: %s", error.f_str());
        return std::nullopt;
    }

    if (!doc["ip"].is<String>())
    {
        log_i("JSON invalid format %s", aData.c_str());
        return std::nullopt;
    }

    Coordinates coord;
    // Fetch values
    coord.latitude = doc["latitude"].as<double>();
    coord.longitude = doc["longitude"].as<double>();

    return std::make_optional<Coordinates>(std::move(coord));
}

auto GetApiUrl(const String& aApiKey)-> String {
    return String("https://api.ipgeolocation.io/ipgeo")
            + "?apiKey="
            + aApiKey;
}

auto GetLocationCoordinates(const String &aApiKey) -> bool
{
    const String serverPath = GetApiUrl(aApiKey);
    log_d("%s", serverPath.c_str());

    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath);

    // [ ]FIXME:remove
    // If you need server authentication, insert user and password below
    // http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Send HTTP GET request
    int httpResponseCode = http.GET();
    if (httpResponseCode <= 0)
    {
        log_w("Error code: %d", httpResponseCode);
        // Free resources
        http.end();

        return false;
    }

    // [ ]TODO: handle response codes and JSONs 200 401 404
    log_d("HTTP Response code: %d", httpResponseCode);

    const String payload = http.getString();
    // log_d("%s", payload.c_str());

    const auto res = ParseJson(payload);

    // Free resources
    http.end();

    if (!res)
        return false;

    Coordinates_ = res.value();
    // Print values.
    log_i("Acquired coordinates: [%3.3f, %3.3f]", Coordinates_.latitude, Coordinates_.longitude);

    return true;
}