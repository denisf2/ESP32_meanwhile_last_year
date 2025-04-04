#include "OpenMeteoApi.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <optional>

#include "timeaux.h"

WeatherHistory_t weatherHistory;

auto ParseJsonOpmet(const String& aData) -> std::optional<WeatherHistory_t>
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

    const int code = doc["generationtime_ms"].as<double>();

    if (!(respondOK == code || respondGoodKeyInvalidData == code))
        return std::nullopt;

    // Fetch values.
    JsonObject daily = doc["daily"];
    JsonArray days = daily["time"];
    JsonArray Tmax = daily["temperature_2m_max"];
    JsonArray Tmin = daily["temperature_2m_min"];

    auto size = days.size();
    if(days < size)
        log_w("Recieved info for %d days", size);
    WeatherHistory_t w;
    for(auto i{0u}; i < days; ++i)
    {
        w.points[i].Tmax = Tmax[i].as<double>();
        w.points[i].Tmin = Tmin[i].as<double>();
        w.points[i].days = days[i].as<const char*>();
    }

    return std::make_optional(std::move(w));
}

auto GetApiUrl(const String &aLat, const String &aLon, unsigned long aSinceEpoch) -> String
{
    auto [begin, end] = GetDateRangeEnds(aSinceEpoch);
    constexpr char requestedParams[]{"temperature_2m_max,temperature_2m_min"};
    return String("https://historical-forecast-api.open-meteo.com/v1/forecast")
                + "?latitude=" + aLat
                + "&longitude=" + aLon
                + "&start_date=" + begin //aDate -3 days
                + "&end_date=" + end  //aDate +3 days
                + "&daily=" + requestedParams;
                // + "&timezone=" + "Europe%2FBerlin";
}

auto GetWeatherHistory(const String &aLat, const String &aLon, unsigned long aSinceEpoch) -> bool
{
    const String serverPath = GetApiUrl(aLat, aLon, aSinceEpoch);
    log_d("%s", serverPath.c_str());

    WiFiClientSecure client;
    // Skip certificate verification only in dev
    client.setInsecure();

    HTTPClient https;
    // Your Domain name with URL path or IP address with path
    https.begin(client, serverPath.c_str());

    // If you need server authentication, insert user and password below
    // https.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Send HTTP GET request
    int httpResponseCode = https.GET();

    if (httpResponseCode <= 0)
    {
        log_d("Error code: %d", httpResponseCode);
        https.end();
        return false;
    }

    // [ ]TODO: handle response codes and JSONs 200 400 401 404
    log_d("HTTP Response code: %d", httpResponseCode);
    const String payload = https.getString();
    // log_d("%s", payload.c_str());

    auto res = ParseJsonOpmet(payload);

    // Free resources
    https.end();

    if(!res)
        return false;

    weatherHistory = res.value();

    // Print values.
    // log_i("Acquired temperature: [ %4.1f ]", weather.temp);

    return true;
}