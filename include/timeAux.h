#ifndef TIMEAUX_H__
#define TIMEAUX_H__

#include <Arduino.h>

auto GetDateRangeEnds(unsigned long aEpochTime) -> std::pair<String, String>;

#endif