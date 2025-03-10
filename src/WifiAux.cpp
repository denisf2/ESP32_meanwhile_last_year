#include "WifiAux.h"

auto to_string(const wifi_auth_mode_t aMode) -> String {
    switch (aMode)
    {
    case WIFI_AUTH_OPEN:
        return String("open");
    case WIFI_AUTH_WEP:
        return String("WEP");
    case WIFI_AUTH_WPA_PSK:
        return String("WPA");
    case WIFI_AUTH_WPA2_PSK:
        return String("WPA2");
    case WIFI_AUTH_WPA_WPA2_PSK:
        return String("WPA+WPA2");
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return String("WPA2-EAP");
    case WIFI_AUTH_WPA3_PSK:
        return String("WPA3");
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return String("WPA2+WPA3");
    case WIFI_AUTH_WAPI_PSK:
        return String("WAPI");
    default:
        return String("unknown");
    }
};