#ifndef WIFIAUX_H__
#define WIFIAUX_H__

#include <WiFi.h>

auto to_string(const wifi_auth_mode_t aMode) -> String;
auto LogPrintWiFiAPsPrettyTable(WiFiClass& aWiFi, const int16_t aTotal) -> void;

#endif /*WIFIAUX_H__*/