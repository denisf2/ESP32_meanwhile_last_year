#include "IpGeolocationApi.h"
#include "HttpClientAux.h"

#include <ArduinoJson.h>

#include <optional>

/*
    api description: https://ipgeolocation.io/ip-location-api.html#documentation-overview
*/

namespace IpGeo
{
constexpr char TAG[] = "[IpGeoApi]";
constexpr char API_BASE_URL[] = "https://api.ipgeolocation.io/ipgeo";

Coordinates_t coordinates;

auto ParseJsonResponse(const String &aData) -> std::optional<Coordinates_t>
{
    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    // Test if parsing succeeds.
    if (DeserializationError error = deserializeJson(doc, aData); error)
    {
        log_w("%s JSON deserializition is failed. Error code: %s", TAG, error.f_str());
        return std::nullopt;
    }

    if (!doc["ip"].is<String>())
    {
        log_i("%s JSON invalid format %s", TAG, aData.c_str());
        return std::nullopt;
    }

    return Coordinates_t {
        doc["latitude"].as<double>()
        , doc["longitude"].as<double>()
    };
}

auto BuildApiUrl(const String& aApiKey)-> String
{
    return String(API_BASE_URL) + "?apiKey=" + aApiKey;
}

auto FetchData(const String &aApiKey) -> bool
{
    const String requestUrl = BuildApiUrl(aApiKey);
    log_d("%s", requestUrl.c_str());

    const auto respond = SendGetRequest(requestUrl);
    if(!respond)
        return false;

    const auto res = ParseJsonResponse(respond.value());
    if (!res)
        return false;

    coordinates = res.value();
    // Print values.
    log_i("%s Acquired coordinates: [%3.3f, %3.3f]", TAG, coordinates.latitude, coordinates.longitude);

    return true;
}

} // namespace