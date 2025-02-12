#ifndef NVSPREFERENCES_H_
#define NVSPREFERENCES_H_

#include <Preferences.h>

enum class SettingsType
{
    user
    , factory
};

auto RestoreStoredData() -> void;

auto SaveIpGeolocation(const String &aValue) -> void;
auto SaveOpenWeather(const String &aValue) -> void;
auto SaveWifiSSID(const String &aValue) -> void;
auto SaveWifiPassword(const String &aValue) -> void;

auto GetIpGeoKey() -> String;
auto GetOpenWeatherKey() -> String;
auto GetWifiSSID(SettingsType aType = SettingsType::user) -> String;
auto GetWiFiPassword(SettingsType aType = SettingsType::user) -> String;
auto IsColdStart() -> bool;

extern Preferences nvsPrefs;

#endif /* NVSPREFERENCES_H_ */