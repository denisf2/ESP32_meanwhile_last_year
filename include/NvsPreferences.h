#ifndef NVSPREFERENCES_H_
#define NVSPREFERENCES_H_

#include <Preferences.h>

auto RestoreStoredData() -> void;

auto SaveIpGeolocation(const String &aValue) -> void;
auto SaveOpenWeather(const String &aValue) -> void;
auto SaveWifiSSID(const String &aValue) -> void;
auto SaveWifiPassword(const String &aValue) -> void;

auto GetIpGeoKey() -> String;
auto GetOpenWeatherKey() -> String;
auto GetWifiSSID() -> String;
auto GetWiFiPassword() -> String;

extern Preferences nvsPrefs;

#endif /* NVSPREFERENCES_H_ */