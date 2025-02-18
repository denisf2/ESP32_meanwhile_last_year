#ifndef OPENWEATHERAPI_H__
#define OPENWEATHERAPI_H__

#include <Arduino.h>

auto GetForecast(const String &aApiKey, const String &aLat, const String &aLon) -> bool;

#endif /* OPENWEATHERAPI_H__ */