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

    auto FetchData(const String &aLat, const String &aLon) -> bool;

    extern std::optional<WeatherHistory_t> weatherHistory;
    extern std::optional<WeatherHistory2_t> weatherWeek;
} // namespace

#endif /*OPENMETEOAPI_H__*/