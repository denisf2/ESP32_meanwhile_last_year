#include "OpenMeteoApi.h"
#include "HttpClientAux.h"
#include "timeaux.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <optional>
#include <functional>

const char TAG[] = "[OpenMeteoApi]";

WeatherHistory_t weatherHistory;
WeatherHistory2_t weatherWeek;
bool chartHistoryDataReady{false};
bool chartWeekDataReady{false};

auto ParseHistory(JsonDocument& aJson) -> WeatherHistory_t
{
    // Fetch values
    JsonObject daily = aJson["daily"];
    JsonArray time = daily["time"];
    JsonArray Tmax = daily["temperature_2m_max"];
    JsonArray Tmin = daily["temperature_2m_min"];

    auto size = time.size();
    if(days < size)
        log_i("%s Recieved info for %d days", TAG, size);

    WeatherHistory_t weatherData;
    for(auto i{0u}; i < std::min(days, size); ++i)
    {
        weatherData.points[i].Tmax = Tmax[i].as<double>();
        weatherData.points[i].Tmin = Tmin[i].as<double>();
        weatherData.points[i].days = time[i].as<const char*>();
    }

    return weatherData;
}

auto ParseLastWeek(JsonDocument& aJson) -> WeatherHistory2_t
{
    // Fetch values
    JsonObject daily = aJson["daily"];
    JsonArray time = daily["time"];
    JsonArray Tmean = daily["temperature_2m_mean"];

    auto size = time.size();
    if(days < size)
        log_i("%s Recieved info for %d days", TAG, size);

    WeatherHistory2_t weatherData;
    for(auto i{0u}; i < std::min(days, size); ++i)
    {
        weatherData.points[i].Tmean = Tmean[i].as<double>();
        weatherData.points[i].days = time[i].as<const char*>();
    }

    weatherData.current = aJson["current"]["temperature_2m"].as<double>();

    return weatherData;
}

template<typename T>
auto ParseJsonOpmet(const String& aData
                    , std::function<auto(JsonDocument& aJson) -> T> aParse
                    ) -> std::optional<T>
{
    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, aData);

    // Test if parsing succeeds.
    if (error)
    {
        log_w("%s JSON deserialition failed. Error code: %s", TAG, error.c_str());
        return std::nullopt;
    }

    const auto hasError = doc["error"];
    if (hasError.is<bool>() && hasError.as<bool>())
    {
        const auto reason = doc["reason"];
        if (reason.is<String>())
            log_w("%s OpenMeteo.com API request failed. The reason is \"%s\"", TAG, reason.as<String>().c_str());

        return std::nullopt;
    }

    if (!doc["generationtime_ms"].is<double>())
    {
        log_w("%s OpenMeteo.com API request failed. Invalid JSON format", TAG);
        return std::nullopt;
    }

    auto weatherData = aParse(doc);

    return std::make_optional(std::move(weatherData));
}

auto GetWeekApiUrl(const String &aLat, const String &aLon) -> String
{
    const auto [begin, end] = GetDateRangeEnds(false);
    constexpr char dailyParams[]{"temperature_2m_mean"};
    constexpr char currentParams[]{"temperature_2m"};
    return String("https://api.open-meteo.com/v1/forecast")
                    + "?latitude=" + aLat
                    + "&longitude=" + aLon
                    + "&start_date=" + begin
                    + "&end_date=" + end
                    + "&daily=" + dailyParams
                    + "&current=" + currentParams;
}

auto GetHistoryApiUrl(const String &aLat, const String &aLon) -> String
{
    const auto [begin, end] = GetDateRangeEnds(true);
    constexpr char dailyParams[]{"temperature_2m_max,temperature_2m_min"};
    return String("https://historical-forecast-api.open-meteo.com/v1/forecast")
                    + "?latitude=" + aLat
                    + "&longitude=" + aLon
                    + "&start_date=" + begin
                    + "&end_date=" + end
                    + "&daily=" + dailyParams;
                    // + "&timezone=" + "Europe%2FBerlin";
}

template<typename T>
auto GetWeatherForPeriod(const String &aLat, const String &aLon
                            , std::function<auto(const String &aLat, const String &aLon) -> String> aGetURL
                            , std::function<auto(JsonDocument& aJson) -> T> aParser
                        ) -> bool
{
    const String requestUrl = aGetURL(aLat, aLon);
    log_d("%s %s", TAG, requestUrl.c_str());

    auto respond = SendGetRequest(requestUrl);
    if(!respond)
        return false;

    auto res = ParseJsonOpmet<T>(respond.value(), aParser);
    if(!res)
        return false;

    if constexpr (std::is_same_v<T, WeatherHistory_t>)
        weatherHistory = res.value();
    else
        weatherWeek = res.value();

    return true;
}

auto GetWeatherLastYear(const String &aLat, const String &aLon) -> bool
{
    if(GetWeatherForPeriod<WeatherHistory_t>(aLat, aLon, GetHistoryApiUrl, ParseHistory))
    {
        log_d("%s Acquired history temperature", TAG);
        chartHistoryDataReady = true;

        return true;
    }

    return false;
}

auto GetWeatherLastWeek(const String &aLat, const String &aLon) -> bool
{
    if(GetWeatherForPeriod<WeatherHistory2_t>(aLat, aLon, GetWeekApiUrl, ParseLastWeek))
    {
        log_d("%s Acquired week temperature", TAG);
        chartWeekDataReady = true;
        return true;
    }

    return false;
}
