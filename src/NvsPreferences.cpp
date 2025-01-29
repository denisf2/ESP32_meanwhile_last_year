#include "NvsPreferences.h"
#include "TLog.h"

Preferences nvsPrefs;

auto RestoreStoredData() -> void 
{
  constexpr bool RO_MODE = true;
  constexpr bool RW_MODE = false;

  TLog::println("Loading stored data");
  nvsPrefs.begin("AppNamespace", RO_MODE);

  if(false == nvsPrefs.isKey("nvsInit"))
  {
    nvsPrefs.end();
    TLog::println("Restore default values : ");  
    nvsPrefs.begin("AppNamespace", RW_MODE);

    //init storage by default values
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
