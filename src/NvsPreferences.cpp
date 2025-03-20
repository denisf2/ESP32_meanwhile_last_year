#include "NvsPreferences.h"

Preferences nvsPrefs;

constexpr bool RO_MODE = true;
constexpr bool RW_MODE = false;

const char defaultValuesKey[]{"nvsInit"};
const char AppNamespace[]{"AppNamespace"};

const char varNameOpenWeatherKey[]{"OpenWeather"};
const char varNameIpGeolocationKey[]{"ipGeolocation"};
const char varNameWiFiSsid[]{"wifiSSID"};
const char varNameWiFiPassword[]{"wifiPassword"};

String ip2geo;
String opwthr;
String wifiSSID;
String wifiPassword;

const char defaultSSID[] = "esp32";
const char defaultPass[] = "esp32pass";

auto RestoreDefaultData() -> void
{
    log_i("Restoring default values");
    nvsPrefs.begin(AppNamespace, RW_MODE);

    // init storage by default values
    // https://api.openweathermap.org
    nvsPrefs.putString(varNameOpenWeatherKey, "");
    // https://api.ipgeolocation.io/ipgeo
    nvsPrefs.putString(varNameIpGeolocationKey, "");
    nvsPrefs.putString(varNameWiFiSsid, defaultSSID);
    nvsPrefs.putString(varNameWiFiPassword, defaultPass);

    nvsPrefs.putBool(defaultValuesKey, true);

    nvsPrefs.end();
    log_d("Restoring default values has been done");
}

auto RestoreStoredData() -> void
{
    log_i("Loading stored data");
    nvsPrefs.begin(AppNamespace, RO_MODE);

    if (false == nvsPrefs.isKey(defaultValuesKey))
    {
        nvsPrefs.end();
        RestoreDefaultData();
        nvsPrefs.begin(AppNamespace, RO_MODE);
    }

    // store in app values
    opwthr = nvsPrefs.getString(varNameOpenWeatherKey);
    ip2geo = nvsPrefs.getString(varNameIpGeolocationKey);
    wifiSSID = nvsPrefs.getString(varNameWiFiSsid);
    wifiPassword = nvsPrefs.getString(varNameWiFiPassword);

    nvsPrefs.end();
}

auto Save(const String &aKey, const String &aValue) -> void
{
    log_d("Saving [%s, %s]", aKey.c_str(), aValue.c_str());

    nvsPrefs.putString(aKey.c_str(), aValue.c_str());
    nvsPrefs.begin(AppNamespace, RW_MODE);
    nvsPrefs.end();
}

auto SaveIpGeolocation(const String &aValue) -> void
{
    Save(varNameIpGeolocationKey, aValue);
    ip2geo = aValue;
}

auto SaveOpenWeather(const String &aValue) -> void
{
    Save(varNameOpenWeatherKey, aValue);
    opwthr = aValue;
}

auto SaveWifiSSID(const String &aValue) -> void
{
    Save(varNameWiFiSsid, aValue);
    wifiSSID = aValue;
}

auto SaveWifiPassword(const String &aValue) -> void
{
    Save(varNameWiFiPassword, aValue);
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