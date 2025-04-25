#ifndef OPENMETEOAPI_H__
#define OPENMETEOAPI_H__

#include <Arduino.h>

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

auto GetWeatherHistory(const String &aLat, const String &aLon, unsigned long aSinceEpoch) -> bool;

extern WeatherHistory_t weatherHistory;
extern bool chartDataReady;

#endif /*OPENMETEOAPI_H__*/