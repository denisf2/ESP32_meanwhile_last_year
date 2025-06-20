#include "WifiAux.h"
#include "JsonAux.h"
#include "NvsPreferences.h"

constexpr char TAG[] = "[WiFi]";
bool scanInProgress{false};

auto to_string(const wifi_auth_mode_t aMode) -> String
{
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
}

auto LogPrintWiFiAPsPrettyTable(WiFiClass &aWiFi, const int16_t aTotal) -> void
{
    log_i("%s Nr | SSID                             | RSSI | CH | Encryption", TAG);
    for (auto i = 0; i < aTotal; ++i)
    {
        // Print SSID and RSSI for each network found
        log_i("%s %2d | %-32.32s | %4d | %2d | %s"
                , TAG
                , i + 1
                , aWiFi.SSID(i).c_str()
                , aWiFi.RSSI(i)
                , aWiFi.channel(i)
                , to_string(aWiFi.encryptionType(i)).c_str()
            );
    }
}

// blocks call site to scan WiFi
auto ScanWiFiAPsJSON(WiFiClass &aWiFi) -> String
{
    // WiFi.scanNetworks will return the number of networks found.
    const auto n = aWiFi.scanNetworks();
    log_i("%s Scan done", TAG);
    if (0 == n)
        log_i("%s No networks found", TAG);
    else
    {
        log_i("%s Networks found: %d", TAG, n);
        LogPrintWiFiAPsPrettyTable(aWiFi, n);

        const auto json = WiFiAPtoJSON(aWiFi, n);
        // [x]TODO: wrap into JSON

        // Delete the scan result to free memory for code below.
        aWiFi.scanDelete();

        return json;
    }

    // Delete the scan result to free memory for code below.
    aWiFi.scanDelete();
    return {};
}

auto StartWiFiScanAsync(WiFiClass &aWiFi) -> void
{
    // [x]TODO: run in async mode.
    aWiFi.scanNetworks(true); // to start detached
    scanInProgress = true;
}

// must call the function time to time in the main loop
auto CheckWiFiScan(WiFiClass &aWiFi) -> std::optional<String>
{
    const auto n = aWiFi.scanComplete(); // to check periodically
    if (0 == n)
    {
        log_i("%s No networks found", TAG);
    }
    else if (n > 0)
    {
        // "finished"
        log_i("%s Networks found: %d", TAG, n);
        LogPrintWiFiAPsPrettyTable(aWiFi, n);

        const auto json = WiFiAPtoJSON(aWiFi, n);

        // Delete the scan result to free memory for code below.
        aWiFi.scanDelete();

        scanInProgress = false;

        return std::make_optional(std::move(json));
    }
    else if(WIFI_SCAN_RUNNING == n)
    {
        // "in progress"
        // [ ]TODO: how long will it take?
        // [ ]TODO: when do we need release resources?
        // aWiFi.scanDelete();
        log_i("%s Scanning...", TAG);
    }
    else if(WIFI_SCAN_FAILED == n)
    {
        // scanInProgress = false;
        log_w("%s Scanning failed", TAG);
    }

    return {};
}

auto SetupWiFiAccessPoint(WiFiClass &aWiFi) -> void
{
    const auto ssid = GetWifiSSID(SettingsType::factory);
    const auto pass = GetWiFiPassword(SettingsType::factory);
    log_i("%s Setting default AP (Access Point) ssid: %s, pass: %s", TAG, ssid, pass);
    aWiFi.mode(wifi_mode_t::WIFI_MODE_AP);
    aWiFi.disconnect();
    ScanWiFiAPsJSON(aWiFi);
    aWiFi.softAP(ssid, pass);

    const auto ip = aWiFi.softAPIP();
    log_i("%s AP IP address: %s", TAG, ip.toString().c_str());
}

auto LockingWiFiConnection(WiFiClass aWiFi) -> bool
{
    const auto wifiSSID = GetWifiSSID();
    const auto wifiPass = GetWiFiPassword();

    // Connect to Wi-Fi network with SSID and password
    constexpr uint32_t WIFI_RECON_DELAY_MILLISEC{10000};
    constexpr uint32_t WIFI_PROGRESS_DELAY_MILLISEC{500};

    auto status = wl_status_t::WL_IDLE_STATUS;
    for (int count{0}; count < 2 && wl_status_t::WL_CONNECTED != status; ++count)
    {
        log_i("%s Attempting to connect to SSID: %s", TAG, wifiSSID.c_str());

        status = aWiFi.begin(wifiSSID, wifiPass);
        for(int split{0}
                ; wl_status_t::WL_CONNECTED != status && split < 10
                ; ++split, status = aWiFi.status())
        {
            log_i("%s Waiting", TAG);
            delay(WIFI_PROGRESS_DELAY_MILLISEC);
        }

        if (wl_status_t::WL_CONNECTED != status)
            delay(WIFI_RECON_DELAY_MILLISEC);
        else
            return true;
    }

    return false;
}

auto PrintWifiStatus(WiFiClass &aWiFi) -> void
{
    // Print local IP address and start web server
    log_i("%s WiFi status: Connected to SSID: %s IP Address: %s Signal strength (RSSI): %d dBm"
            , TAG
            , aWiFi.SSID().c_str()
            , aWiFi.localIP().toString().c_str()
            , aWiFi.RSSI()
        );
}