#ifndef TIMEAUX_H__
#define TIMEAUX_H__

#include <Arduino.h>
#include <ctime>

extern const char ntpServer1[];
extern const char ntpServer2[];
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern const char time_zone[];

// Callback function (gets called when time adjusts via NTP)
auto TimeAvailable(timeval * aTimeVal) -> void;
auto GetDateRangeEnds(bool aForLastYear = true) -> std::pair<String, String>;

#endif