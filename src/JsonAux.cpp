#include "JsonAux.h"
#include "WifiAux.h"
#include "NvsPreferences.h"
#include "OpenMeteoApi.h"
#include "OpenWeatherApi.h"

const char TAG[] = "[JSON]";

auto WiFiAPtoJSON(WiFiClass& aWiFi, const int16_t aTotal) -> String
{
    JsonDocument doc;
    doc["message"] = "wifiAps";
    auto APs = doc["APs"];
    for (auto i = 0; i < aTotal; ++i)
    {
        auto AP = APs.add<JsonObject>();
        AP["ssid"] = aWiFi.SSID(i).c_str();
        AP["rssi"] = aWiFi.RSSI(i);
        AP["channel"] = aWiFi.channel(i);
        AP["encryption"] = to_string(aWiFi.encryptionType(i)).c_str();
    }

    String serial;
    serializeJson(doc, serial);
    log_d("%s Available WiFi APs response json \r\n%s", TAG, serial.c_str());

    return serial;
}

auto SerializeRespondJSON(JsonDocument&& aDoc, const String& aMsgType, bool aValidated) -> String
{
    // drop request json
    aDoc.clear();

    aDoc["message"] = aMsgType.c_str();
    aDoc["valid"] = (aValidated) ? "true" : "false";

    String serial;
    serializeJson(aDoc, serial);
    log_d("%s Check response json %s", TAG, serial.c_str());

    return serial;
}

auto SerializeFormStoredData(JsonDocument&& aDoc, const String& aMsgType) -> String
{
    // [x]TODO: rename and implement
    // [ ]TODO: need interface refactoring

    // drop request json
    aDoc.clear();

    aDoc["message"] = "FormFillStoredData";
    aDoc["WifiSsid"] = GetWifiSSID();
    // [ ]TODO: need to think about this
    // aDoc["WiFiPassword"] = GetWiFiPassword();
    aDoc["WiFiPassword"] = "";
    aDoc["Longitude"] = GetLongitude();
    aDoc["Latitude"] = GetLatitude();
    aDoc["IpGeolocKey"] = GetIpGeoKey();
    aDoc["OpenWeatherKey"] = GetOpenWeatherKey();

    String serial;
    serializeJson(aDoc, serial);
    log_d("%s Check response json %s", TAG, serial.c_str());

    return serial;
}

auto GetChartData() -> String
{
    JsonDocument doc;
    doc["message"] = "ChartDataResponse";

    auto daily = doc["daily"].to<JsonObject>();
    JsonArray time = daily["time"].to<JsonArray>();
    JsonArray Tmax = daily["temperature_2m_max"].to<JsonArray>();
    JsonArray Tmin = daily["temperature_2m_min"].to<JsonArray>();

    for(auto i = 0; i < days; ++i)
    {
        Tmax.add(weatherHistory.points[i].Tmax);
        Tmin.add(weatherHistory.points[i].Tmin);
        time.add(weatherHistory.points[i].days);
    }

    auto current = doc["current"].to<JsonArray>();

    const size_t lastDays{3};
    for(auto i = 0; i < lastDays; ++i)
        current.add(weatherWeek.points[i].Tmean);

    current.add(weatherWeek.current);

    String serial;
    serializeJson(doc, serial);
    log_d("%s Collected chart data \r\n%s", TAG, serial.c_str());

    return serial;
}
