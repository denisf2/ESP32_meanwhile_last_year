#include "OpenMeteoApi.h"
#include "HttpClientAux.h"
#include "timeaux.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <optional>
#include <functional>

#include "functional_aux.h"

namespace OMeteo
{
constexpr char TAG[] = "[OpenMeteoApi]";
constexpr char BASE_API_URL[] = "https://api.open-meteo.com/v1/forecast";
constexpr char HISTORICAL_API_URL[] = "https://historical-forecast-api.open-meteo.com/v1/forecast";

std::optional<WeatherHistory_t> weatherHistory{std::nullopt};
std::optional<WeatherHistory2_t> weatherWeek;

auto DecomposeHistoryJson(JsonDocument&& aJson) -> WeatherHistory_t
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

auto DecomposeLastWeekJson(JsonDocument&& aJson) -> WeatherHistory2_t
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

auto ValidateJsonResponse(String&& aData) -> Functional::Optional<JsonDocument>
{
    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    // Test if parsing succeeds.
    if (DeserializationError error = deserializeJson(doc, aData); error)
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

    return doc;
}

auto BuildWeekApiUrl(const String &aLat, const String &aLon) -> String
{
    const auto [begin, end] = GetDateRangeEnds(false);
    constexpr char dailyParams[]{"temperature_2m_mean"};
    constexpr char currentParams[]{"temperature_2m"};
    return String(BASE_API_URL)
                    + "?latitude=" + aLat
                    + "&longitude=" + aLon
                    + "&start_date=" + begin
                    + "&end_date=" + end
                    + "&daily=" + dailyParams
                    + "&current=" + currentParams;
}

auto BuildHistoryApiUrl(const String &aLat, const String &aLon) -> String
{
    const auto [begin, end] = GetDateRangeEnds(true);
    constexpr char dailyParams[]{"temperature_2m_max,temperature_2m_min"};
    return String(HISTORICAL_API_URL)
                    + "?latitude=" + aLat
                    + "&longitude=" + aLon
                    + "&start_date=" + begin
                    + "&end_date=" + end
                    + "&daily=" + dailyParams;
                    // + "&timezone=" + "Europe%2FBerlin";
}

template<typename T>
using JsonParser = std::function<auto(JsonDocument&& aJson) -> T>;
using UrlBuilder = std::function<auto(const String &aLat, const String &aLon) -> String>;

template<typename T>
auto GetWeatherForPeriod(const String &aLat
                        , const String &aLon
                        , UrlBuilder aGetURL
                        , JsonParser<T> aParser)
                    -> std::optional<T>
{
    const String requestUrl = aGetURL(aLat, aLon);
    log_d("%s %s", TAG, requestUrl.c_str());

    return Functional::Optional(SendGetRequest(requestUrl))
                                .and_then(ValidateJsonResponse)
                                .transform(aParser);
}

auto GetWeatherLastYear(const String &aLat, const String &aLon) -> bool
{
    if(auto res = GetWeatherForPeriod<WeatherHistory_t>(aLat, aLon, BuildHistoryApiUrl, DecomposeHistoryJson); res)
    {
        log_d("%s Acquired history temperature", TAG);
        weatherHistory = res;
        return true;
    }

    return false;
}

auto GetWeatherLastWeek(const String &aLat, const String &aLon) -> bool
{
    if(auto res = GetWeatherForPeriod<WeatherHistory2_t>(aLat, aLon, BuildWeekApiUrl, DecomposeLastWeekJson); res)
    {
        log_d("%s Acquired week temperature", TAG);
        weatherWeek = res;
        return true;
    }

    return false;
}

auto FetchData(const String &aLat, const String &aLon) -> bool
{
    // last year
    return OMeteo::GetWeatherLastYear(aLat, aLon)
    // last three days
        && OMeteo::GetWeatherLastWeek(aLat, aLon);
}
} //namespace