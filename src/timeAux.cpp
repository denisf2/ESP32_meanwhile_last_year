#include "timeAux.h"

#include <ctime>

auto GetDateRangeEnds(unsigned long aEpochTime) -> std::pair<String, String>
{
    // !!! reducing type
    // cannot see any other ways to obtain time
    tm* timeInfo = std::localtime(reinterpret_cast<const time_t* >(&aEpochTime));
    if(nullptr == timeInfo)
        return {"Invalid time value", "Invalid time value"};

    constexpr time_t oneDayInSec = 86400;

    time_t now = mktime(timeInfo) - 365 * oneDayInSec;

    // Calculate -3 days
    time_t minus3 = now - 3 * oneDayInSec;
    tm minus3Days = *localtime(&minus3);

    // Calculate +3 days
    time_t plus3 = now + 3 * oneDayInSec;
    tm plus3Days = *localtime(&plus3);

    // Format dates
    auto formatDate = [](const tm &date)
    {
        char buf[11]{};
        snprintf(buf, sizeof(buf)
                    , "%04d-%02d-%02d"
                    , date.tm_year + 1900
                    , date.tm_mon + 1
                    , date.tm_mday);
        return String(buf);
    };

    const auto begin = formatDate(minus3Days);
    const auto end = formatDate(plus3Days);

    log_d("Date range is from %s till %s", begin.c_str(), end.c_str());

    return {std::move(begin), std::move(end)};
}