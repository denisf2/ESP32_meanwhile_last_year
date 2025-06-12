#include "IpGeolocationApi.h"
#include "HttpClientAux.h"

#include <ArduinoJson.h>

#include <optional>

#include "functional_aux.h"

/*
    api description: https://ipgeolocation.io/ip-location-api.html#documentation-overview
*/

namespace IpGeo
{
constexpr char TAG[] = "[IpGeoApi]";
constexpr char API_BASE_URL[] = "https://api.ipgeolocation.io/ipgeo";

std::optional<Coordinates_t> coordinates;

auto DecomposeJson(JsonDocument&& aDoc) -> Coordinates_t
{
    return Coordinates_t {
        aDoc["latitude"].as<double>()
        , aDoc["longitude"].as<double>()
    };
}

auto ValidateJsonResponse(const String &aData) -> Functional::Optional<JsonDocument>
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

    return doc;
}

auto BuildApiUrl(const String& aApiKey)-> String
{
    return String(API_BASE_URL) + "?apiKey=" + aApiKey;
}

auto FetchData(const String &aApiKey) -> bool
{
    const String requestUrl = BuildApiUrl(aApiKey);
    log_d("%s", requestUrl.c_str());

    if (auto res = Functional::Optional(SendGetRequest(requestUrl))
                                        .and_then(ValidateJsonResponse)
                                        .transform(DecomposeJson)
                                        .into_optional()
        ; res)
    {
        coordinates = res;
        // Print values.
        log_i("%s Acquired coordinates: [%3.3f, %3.3f]", TAG, coordinates.value().latitude, coordinates.value().longitude);

        return true;
    }

    return false;
}

} // namespace