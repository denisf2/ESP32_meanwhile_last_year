#ifndef OPENWEATHERAPI_H__
#define OPENWEATHERAPI_H__

#include <Arduino.h>

namespace OpenWeather
{
    struct Weather_t
    {
        double temp{0.0};
    };

    auto FetchData(const String &aApiKey, const String &aLat, const String &aLon) -> bool;

    extern std::optional<Weather_t> weather;
}

#endif /* OPENWEATHERAPI_H__ */