#include "NvsPreferences.h"
#include "TLog.h"

Preferences nvsPrefs;

constexpr bool RO_MODE = true;
constexpr bool RW_MODE = false;

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
    nvsPrefs.putString("wifiSSID", "");
    nvsPrefs.putString("wifiPassword", "");

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
    auto opwTmp = nvsPrefs.getString("OpenWeather");
    auto ip2geo = nvsPrefs.getString("ipGeolocation");
    // [x]TODO: add default wifi ssid and wifi pass values
    auto wifiSSID = nvsPrefs.getString("wifiSSID");
    auto wifiPassword = nvsPrefs.getString("wifiPassword");

    nvsPrefs.end();
}

auto Save(const String &aKey, const String &aValue) -> void
{
    nvsPrefs.begin("AppNamespace", RW_MODE);
    nvsPrefs.putString(aKey.c_str(), aValue.c_str());
    nvsPrefs.end();
}

auto SaveIpGeolocation(const String &aValue) -> void
{
    Save("ipGeolocation", aValue);
}

auto SaveOpenWeather(const String &aValue) -> void
{
    Save("OpenWeather", aValue);
}

auto SaveWifiSSID(const String &aValue) -> void
{
    Save("wifiSSID", aValue);
}

auto SaveWifiPassword(const String &aValue) -> void
{
    Save("wifiPassword", aValue);
}
