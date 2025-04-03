#include "JsonAux.h"
#include "WifiAux.h"
#include "NvsPreferences.h"

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
    log_d("Available WiFi APs response json %s", serial.c_str());

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
    log_d("Check response json %s", serial.c_str());

    return serial;
}

auto SerializeFormStoredData(JsonDocument&& aDoc, const String& aMsgType, bool aValidated) -> String
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
    log_d("Check response json %s", serial.c_str());

    return serial;
}

auto PrintWifiStatus(WiFiClass& aWiFi) -> void
{
    // Print local IP address and start web server
    log_i("WiFi status: Connected to SSID: %s  IP Address: %s Signal strength (RSSI): %d dBm"
            , aWiFi.SSID().c_str()
            , aWiFi.localIP().toString().c_str()
            , aWiFi.RSSI()
        );
}