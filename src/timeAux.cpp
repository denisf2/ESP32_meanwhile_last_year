#include "timeAux.h"

// #include <ctime>
#include <iomanip>
#include <sstream>

constexpr char TAG[] = "[Time]";

const char ntpServer1[] = "pool.ntp.org";
const char ntpServer2[] = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// [ ]TODO: TimeZone rule for Europe/Rome including daylight adjustment rules (optional)

// ex. CET-1CEST,M3.5.0,M10.5.0/3:
//     The standard timezone is CET (Central European Time)
//     The offset from UTC is −1
//     The DST timezone is CEST (Central European Summer Time)
//     DST starts at:
//         3: the third month of the year (March)
//         5: the last…
//         0: …Sunday of the month
//         (no time specifier, defaults to 2 AM)
//     DST ends at:
//         10: the tenth month of the year (October)
//         5: the last…
//         0: …Sunday of the month
//         3: at 3 AM
const char time_zone[] = "CET-1CEST,M3.5.0,M10.5.0/3";

auto Format(tm *aTimeInfo, const char *aFormat) -> String
{
    const char *format = aFormat;
    if (!format)
        format = "%c";

    char buf[64]{};
    if (0 == strftime(buf, 64, format, aTimeInfo))
        return {};

    return String(buf);
}

auto PrintLocalTime() -> void
{
    tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        log_i("%s No time available (yet)", TAG);
        return;
    }

    log_i("%s %s", TAG, Format(&timeinfo, "%A, %B %d %Y %H:%M:%S").c_str());
}

auto TimeAvailable(timeval * aTimeVal) -> void
{
    log_i("%s Got time adjustment from NTP!", TAG);
    PrintLocalTime();
}

// Adjust date by a number of days +/-
auto DatePlusDays(struct tm *aDate, int aDays) -> void
{
    constexpr time_t oneDayInSec = 24 * 60 * 60;

    // Seconds since start of epoch
    time_t dateInSec = mktime(aDate) + aDays * oneDayInSec;

    // Update caller's date
    // Use localtime because mktime converts to UTC so may change date
    *aDate = *localtime(&dateInSec);
}

auto GetDateRangeEnds(bool aForLastYear) -> std::pair<String, String>
{
    tm timeInfo;
    if (!getLocalTime(&timeInfo))
        return {"Invalid time value", "Invalid time value"};

    auto timeInfoLastYear = timeInfo;
    // [ ]TODO: solution does not handle leap year. ex: yyyy-02-29
    if (aForLastYear)
        timeInfoLastYear.tm_year -= 1;

    DatePlusDays(&timeInfoLastYear, -3);
    const auto threeDaysPastTodayLastYear = Format(&timeInfoLastYear, "%F");

    DatePlusDays(&timeInfoLastYear
                , (aForLastYear) ? +6 : +3);
    const auto threeDaysBeforeTodayLastYear = Format(&timeInfoLastYear, "%F");

    log_d("%s Date range is from %s till %s"
                , TAG
                , threeDaysPastTodayLastYear.c_str()
                , threeDaysBeforeTodayLastYear.c_str());

    return {std::move(threeDaysPastTodayLastYear)
            , std::move(threeDaysBeforeTodayLastYear)};
}