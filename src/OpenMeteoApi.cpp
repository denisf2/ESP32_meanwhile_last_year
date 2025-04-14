#include "OpenMeteoApi.h"
#include "HttpClientAux.h"
#include "timeaux.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <optional>

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

    auto ress = SendGetRequest(serverPath);
    if(!ress)
        return false;

    auto res = ParseJsonOpmet(ress.value());

    if(!res)
        return false;

    weatherHistory = res.value();

    // Print values.
    // log_i("Acquired temperature: [ %4.1f ]", weather.temp);

    return true;
}