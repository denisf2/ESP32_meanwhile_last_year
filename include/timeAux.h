#ifndef TIMEAUX_H__
#define TIMEAUX_H__

#include <Arduino.h>
#include <ctime>

extern const char ntpServer1[];
extern const char ntpServer2[];
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern const char time_zone[];

auto print(tm *aTimeinfo, const char * aFormat) -> String;
auto printLocalTime() -> void;

// Callback function (gets called when time adjusts via NTP)
auto timeavailable(timeval * aTimeVal) -> void;
auto GetDateRangeEnds(uint64_t aEpochTime) -> std::pair<String, String>;

#endif