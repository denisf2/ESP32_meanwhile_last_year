#include "NvsPreferences.h"

constexpr char TAG[] = "[NVS]";

Preferences nvsPrefs;

constexpr bool RO_MODE = true;
constexpr bool RW_MODE = false;

constexpr char defaultValuesKey[]{"nvsInit"};
constexpr char AppNamespace[]{"AppNamespace"};

constexpr char varNameOpenWeatherKey[]{"OpenWeather"};
constexpr char varNameIpGeolocationKey[]{"ipGeolocation"};
constexpr char varNameLatitude[]{"Latitude"};
constexpr char varNameLongitude[]{"Longitude"};
constexpr char varNameWiFiSsid[]{"wifiSSID"};
constexpr char varNameWiFiPassword[]{"wifiPassword"};
constexpr char varNameAutomaticLocation[]{"AutoLocation"};

String ip2geo;
String opwthr;
String latitude;
String longitude;
String wifiSSID;
String wifiPassword;
bool autoLocation{true};

constexpr char defaultSSID[] = "esp32";
constexpr char defaultPass[] = "esp32pass";

auto RestoreDefaultData() -> void
{
    log_i("%s Restoring default values", TAG);
    nvsPrefs.begin(AppNamespace, RW_MODE);

    // init storage by default values
    // https://api.openweathermap.org
    nvsPrefs.putString(varNameOpenWeatherKey, "");
    // https://api.ipgeolocation.io/ipgeo
    nvsPrefs.putString(varNameIpGeolocationKey, "");
    nvsPrefs.putString(varNameLatitude, "");
    nvsPrefs.putString(varNameLongitude, "");
    nvsPrefs.putString(varNameWiFiSsid, defaultSSID);
    nvsPrefs.putString(varNameWiFiPassword, defaultPass);
    nvsPrefs.putBool(varNameAutomaticLocation, true);

    nvsPrefs.putBool(defaultValuesKey, true);

    nvsPrefs.end();
    log_d("%s Restoring default values has been done", TAG);
}

auto RestoreStoredData() -> void
{
    log_i("%s Loading stored data", TAG);
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
    latitude = nvsPrefs.getString(varNameLatitude);
    longitude = nvsPrefs.getString(varNameLongitude);
    wifiSSID = nvsPrefs.getString(varNameWiFiSsid);
    wifiPassword = nvsPrefs.getString(varNameWiFiPassword);
    autoLocation = nvsPrefs.getBool(varNameAutomaticLocation);

    nvsPrefs.end();
}

auto Save(const char aKey[], const String &aValue) -> void
{
    log_d("%s Saving [%s, %s]", TAG, aKey, aValue.c_str());

    nvsPrefs.begin(AppNamespace, RW_MODE);
    nvsPrefs.putString(aKey, aValue.c_str());
    nvsPrefs.end();
}

auto Save(const char aKey[], bool aValue) -> void
{
    log_d("%s Saving [%s, %s]", TAG, aKey, aValue ? "true" : "false");

    nvsPrefs.begin(AppNamespace, RW_MODE);
    nvsPrefs.putBool(aKey, aValue);
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

auto SaveLatitude(const String &aValue) -> void
{
    Save(varNameLatitude, aValue);
    latitude = aValue;
}

auto SaveLongitude(const String &aValue) -> void
{
    Save(varNameLongitude, aValue);
    longitude = aValue;
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

auto SaveAutoLocation(bool aValue) -> void
{
    Save(varNameAutomaticLocation, aValue);
    autoLocation = aValue;
}

auto GetIpGeoKey() -> String
{
    return ip2geo;
}

auto GetOpenWeatherKey() -> String
{
    return opwthr;
}

auto GetLatitude() -> String
{
    return latitude;
}

auto GetLongitude() -> String
{
    return longitude;
}

auto GetWifiSSID(SettingsType aType) -> String
{
    return {(SettingsType::factory == aType) ? defaultSSID : wifiSSID};
}

auto GetWiFiPassword(SettingsType aType) -> String
{
    return {(SettingsType::factory == aType) ? defaultPass : wifiPassword};
}

auto GetAutoLocation() -> bool
{
    return autoLocation;
}

auto IsColdStart() -> bool
{
    return wifiSSID.equals(defaultSSID) && wifiPassword.equals(defaultPass);
}