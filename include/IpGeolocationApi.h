#ifndef IPGEOLOCATIONAPI_H__
#define IPGEOLOCATIONAPI_H__

#include <Arduino.h>

auto GetLocationCoordinates(const String &aApiKey) -> bool;
struct Coordinates_t
{
    double latitude{0.0};
    double longitude{0.0};
};

extern Coordinates_t coordinates;
#endif /* IPGEOLOCATIONAPI_H__ */