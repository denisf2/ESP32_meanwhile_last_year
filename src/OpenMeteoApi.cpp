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

    const auto hasError = doc["error"];
    if (hasError.is<bool>() && hasError.as<bool>())
    {
        const auto reason = doc["reason"];
        if (reason.is<String>())
            log_w("OpenMeteo.com API request failed. The reason is \"%s\"", reason.as<String>().c_str());

        return std::nullopt;
    }

    if (!doc["generationtime_ms"].is<double>())
    {
        log_w("OpenMeteo.com API request failed. Invalid JSON format");
        return std::nullopt;
    }

    // Fetch values
    JsonObject daily = doc["daily"];
    JsonArray time = daily["time"];
    JsonArray Tmax = daily["temperature_2m_max"];
    JsonArray Tmin = daily["temperature_2m_min"];

    auto size = time.size();
    if(days < size)
        log_i("Recieved info for %d days", size);

    WeatherHistory_t weatherData;
    for(auto i{0u}; i < std::min(days, size); ++i)
    {
        weatherData.points[i].Tmax = Tmax[i].as<double>();
        weatherData.points[i].Tmin = Tmin[i].as<double>();
        weatherData.points[i].days = time[i].as<const char*>();
    }

    return std::make_optional(std::move(weatherData));
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
    const String requestUrl = GetApiUrl(aLat, aLon, aSinceEpoch);
    log_d("%s", requestUrl.c_str());

    auto respond = SendGetRequest(requestUrl);
    if(!respond)
        return false;

    auto res = ParseJsonOpmet(respond.value());
    if(!res)
        return false;

    weatherHistory = res.value();

    // Print values.
    // log_i("Acquired temperature: [ %4.1f ]", weather.temp);
    log_d("Acquired history temperature");

    return true;
}