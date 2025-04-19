#include "timeAux.h"

// #include <ctime>
#include <iomanip>
#include <sstream>

constexpr char TAG[] ="time";

const char ntpServer1[] = "pool.ntp.org";
const char ntpServer2[] = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// [ ]TODO: TimeZone rule for Europe/Rome including daylight adjustment rules (optional)
const char time_zone[] = "CET-1CEST,M3.5.0,M10.5.0/3";

auto print(tm *aTimeInfo, const char *aFormat) -> String
{
    const char *format = aFormat;
    if (!format)
        format = "%c";

    char buf[64]{};
    if (0 == strftime(buf, 64, format, aTimeInfo))
        return {};

    return String(buf);
}

auto printLocalTime() -> void
{
    tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        ESP_LOGI(TAG, "No time available (yet)");
        return;
    }

    ESP_LOGI(TAG, "%s", print(&timeinfo, "%A, %B %d %Y %H:%M:%S").c_str());
}

auto timeavailable(timeval * aTimeVal) -> void
{
    ESP_LOGI(TAG, "Got time adjustment from NTP!");
    printLocalTime();
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

auto GetDateRangeEnds(uint64_t aNTPEpochTime) -> std::pair<String, String>
{
    tm timeInfo;
    if (!getLocalTime(&timeInfo))
        return {"Invalid time value", "Invalid time value"};

    auto timeInfoLastYear = timeInfo;
    // [ ]TODO: solution do not handle leap year. ex: yyyy-02-29
    timeInfoLastYear.tm_year -= 1;

    DatePlusDays(&timeInfoLastYear, -3);
    const auto threeDaysPastTodayLastYear = print(&timeInfoLastYear, "%F");

    DatePlusDays(&timeInfoLastYear, +6);
    const auto threeDaysBeforeTodayLastYear = print(&timeInfoLastYear, "%F");

    ESP_LOGD(TAG, "Date range is from %s till %s"
                , threeDaysPastTodayLastYear.c_str()
                , threeDaysBeforeTodayLastYear.c_str());

    return {std::move(threeDaysPastTodayLastYear)
            , std::move(threeDaysBeforeTodayLastYear)};
}