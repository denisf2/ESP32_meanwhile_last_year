#ifndef OPENWEATHERAPI_H__
#define OPENWEATHERAPI_H__

#include <Arduino.h>

struct Weather_t {
    double temp{0.0};
};

auto GetForecast(const String &aApiKey, const String &aLat, const String &aLon) -> bool;

extern Weather_t weather;

#endif /* OPENWEATHERAPI_H__ */