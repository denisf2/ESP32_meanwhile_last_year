#ifndef IPGEOLOCATIONAPI_H__
#define IPGEOLOCATIONAPI_H__

#include <Arduino.h>

namespace IpGeo
{
    struct Coordinates_t
    {
        double latitude{0.0};
        double longitude{0.0};
    };

    auto FetchData(const String &aApiKey) -> bool;

    extern std::optional<Coordinates_t> coordinates;
} // namespace

#endif /* IPGEOLOCATIONAPI_H__ */