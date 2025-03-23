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
auto SaveLatitude(const String &aValue) -> void;
auto SaveLongitude(const String &aValue) -> void;
auto SaveWifiSSID(const String &aValue) -> void;
auto SaveWifiPassword(const String &aValue) -> void;
auto SaveAutoLocation(bool aValue) -> void;

auto GetIpGeoKey() -> String;
auto GetOpenWeatherKey() -> String;
auto GetLatitude() -> String;
auto GetLongitude() -> String;
auto GetWifiSSID(SettingsType aType = SettingsType::user) -> String;
auto GetWiFiPassword(SettingsType aType = SettingsType::user) -> String;
auto GetAutoLocation() -> bool;

auto IsColdStart() -> bool;

extern Preferences nvsPrefs;

#endif /* NVSPREFERENCES_H_ */