#ifndef WIFIAUX_H__
#define WIFIAUX_H__

#include <WiFi.h>

auto to_string(const wifi_auth_mode_t aMode) -> String;
auto LogPrintWiFiAPsPrettyTable(WiFiClass& aWiFi, const int16_t aTotal) -> void;
auto ScanWiFiAPsJSON(WiFiClass& aWiFi) -> String;
auto SetupWiFiAccessPoint(WiFiClass& aWiFi) -> void;
auto LockingWiFiConnection(WiFiClass aWiFi) -> bool;
auto PrintWifiStatus(WiFiClass& aWiFi) -> void;
auto StartWiFiScanAsync(WiFiClass &aWiFi) -> void;
auto CheckWiFiScan(WiFiClass &aWiFi) -> std::optional<String>;

extern bool scanInProgress;

#endif /*WIFIAUX_H__*/