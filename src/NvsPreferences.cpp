#include "NvsPreferences.h"
#include <TLog.h>

Preferences nvsPrefs;

constexpr bool RO_MODE = true;
constexpr bool RW_MODE = false;

String ip2geo;
String opwthr;
String wifiSSID;
String wifiPassword;

const char defaultSSID[] = "esp32";
const char defaultPass[] = "esp32pass";

auto RestoreDefaultData() -> void //[ ]TODO: why do I need these default values?
{
    TLog::println("Restore default values : ");
    nvsPrefs.begin("AppNamespace", RW_MODE);

    // init storage by default values
    // [x]TODO: add default serveces api key zero values
    // https://api.openweathermap.org
    nvsPrefs.putString("OpenWeather", "");
    // https://api.ipgeolocation.io/ipgeo
    nvsPrefs.putString("ipGeolocation", "");
    // [x]TODO: add default wifi ssid and wifipass zero values
    nvsPrefs.putString("wifiSSID", defaultSSID);
    nvsPrefs.putString("wifiPassword", defaultPass);

    nvsPrefs.putBool("nvsInit", true);

    nvsPrefs.end();
    TLog::print("Done");
}

auto RestoreStoredData() -> void
{
    TLog::println("Loading stored data");
    nvsPrefs.begin("AppNamespace", RO_MODE);

    if (false == nvsPrefs.isKey("nvsInit"))
    {
        nvsPrefs.end();
        RestoreDefaultData();
        nvsPrefs.begin("AppNamespace", RO_MODE);
    }

    // store in app values
    // [ ]TODO: make global variables
    // [x]TODO: restore serveces api key values
    opwthr = nvsPrefs.getString("OpenWeather");
    ip2geo = nvsPrefs.getString("ipGeolocation");
    // [x]TODO: add default wifi ssid and wifi pass values
    wifiSSID = nvsPrefs.getString("wifiSSID");
    wifiPassword = nvsPrefs.getString("wifiPassword");

    nvsPrefs.end();
}

auto Save(const String &aKey, const String &aValue) -> void
{
    TLog::println("Saving[");
    TLog::print(aKey);
    TLog::print(", ");
    TLog::print(aValue);
    TLog::print("]");

    nvsPrefs.begin("AppNamespace", RW_MODE);
    nvsPrefs.putString(aKey.c_str(), aValue.c_str());
    nvsPrefs.end();
}

auto SaveIpGeolocation(const String &aValue) -> void
{
    Save("ipGeolocation", aValue);
    ip2geo = aValue;
}

auto SaveOpenWeather(const String &aValue) -> void
{
    Save("OpenWeather", aValue);
    opwthr = aValue;
}

auto SaveWifiSSID(const String &aValue) -> void
{
    Save("wifiSSID", aValue);
    wifiSSID = aValue;
}

auto SaveWifiPassword(const String &aValue) -> void
{
    Save("wifiPassword", aValue);
    wifiPassword = aValue;
}

auto GetIpGeoKey() -> String
{
    return ip2geo;
}

auto GetOpenWeatherKey() -> String
{
    return opwthr;
}

auto GetWifiSSID(SettingsType aType) -> String
{
    return (SettingsType::factory == aType) ? defaultSSID : wifiSSID;
}

auto GetWiFiPassword(SettingsType aType) -> String
{
    return (SettingsType::factory == aType) ? defaultPass : wifiPassword;
}

auto IsColdStart() -> bool
{
    return defaultSSID == wifiSSID.c_str() && defaultPass == wifiPassword.c_str();
}