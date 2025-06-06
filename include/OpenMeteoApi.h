#ifndef OPENMETEOAPI_H__
#define OPENMETEOAPI_H__

#include <Arduino.h>
namespace OMeteo
{
    constexpr size_t days{7};

    struct WeatherHistory_t
    {
        struct TempPoint
        {
            uint8_t Tmin{0};
            uint8_t Tmax{0};
            String days;
        };

        TempPoint points[days]{};
    };

    // [ ]TODO: eliminate this type
    struct WeatherHistory2_t
    {
        struct TempPoint
        {
            // uint8_t Tmin{0}; < --
            // uint8_t Tmax{0}; < --
            uint8_t Tmean{0}; // < ++
            String days;
        };

        TempPoint points[days]{};
        double current{0.}; // < ++
    };

    auto GetWeatherLastYear(const String &aLat, const String &aLon) -> bool;
    auto GetWeatherLastWeek(const String &aLat, const String &aLon) -> bool;

    extern WeatherHistory_t weatherHistory;
    extern WeatherHistory2_t weatherWeek;
    extern bool chartHistoryDataReady;
    extern bool chartWeekDataReady;
} // namespace

#endif /*OPENMETEOAPI_H__*/