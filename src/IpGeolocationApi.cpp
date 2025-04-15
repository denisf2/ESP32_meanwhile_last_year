#include "IpGeolocationApi.h"
#include "HttpClientAux.h"

#include <ArduinoJson.h>

#include <optional>

Coordinates_t coordinates;

/*
api description: https://ipgeolocation.io/ip-location-api.html#documentation-overview
*/

auto ParseJson(const String &aData) -> std::optional<Coordinates_t>
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

    Coordinates_t coord;
    // Fetch values
    coord.latitude = doc["latitude"].as<double>();
    coord.longitude = doc["longitude"].as<double>();

    return std::make_optional<Coordinates_t>(std::move(coord));
}

auto GetApiUrl(const String& aApiKey)-> String {
    return String("https://api.ipgeolocation.io/ipgeo")
            + "?apiKey="
            + aApiKey;
}

auto GetLocationCoordinates(const String &aApiKey) -> bool
{
    const String requestUrl = GetApiUrl(aApiKey);
    log_d("%s", requestUrl.c_str());

    auto respond = SendGetRequest(requestUrl);
    if(!respond)
        return false;

    const auto res = ParseJson(respond.value());

    if (!res)
        return false;

    coordinates = res.value();
    // Print values.
    log_i("Acquired coordinates: [%3.3f, %3.3f]", coordinates.latitude, coordinates.longitude);

    return true;
}