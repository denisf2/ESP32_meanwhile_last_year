// ======================================================
// Webserver program that gets own coordinates by IP address 
// and acquiring weather forecast info hourly for today and 
// historical weather for several days back.

// Using services:
// https://ipgeolocation.io/
// https://openweathermap.org/

// https://ipgeolocation.io/ip-location-api.html
// https://openweathermap.org/api
// https://openweathermap.org/history
// https://openweathermap.org/api/geocoding-api
// ======================================================

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include <functional>

#include "NvsPreferences.h"
#include "resources.h"
#include <TLog.h>
#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "WebServerHandlers.h"

unsigned long oldmil = 0UL;
unsigned long oldmil2 = 0UL;
const unsigned long UPDATE_INTERVAL_MILLISEC = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};
// ---------------------------------------------------

// ==================================================
// Handle submit form
// ==================================================
void handleSubmit(AsyncWebServerRequest *aRequest)
{
    if (aRequest->hasArg("button1"))
        TLog::println("button1 was pressed");

    SendWebPageResponse(aRequest); // Response to the HTTP request
}

// ===================================================
// Handle root
// ===================================================
void handleRoot(AsyncWebServerRequest *aRequest)
{
    if (aRequest->args())
    {
        handleSubmit(aRequest);
    }
    else
    {
        SendWebPageResponse(aRequest);
    }
}
/*******************
handleRoot->has arguments->handleSubmit->has "button1"->print to serial sensor data
                         |              |              |                          |------>|
                         |              |              |--------------------------------->|
                         |              |------------------------------------------------>|
                         |--------------------------------------------------------------->|
                                                                                          |
onNotFound->handleNotFound                                                                |
                         |--------------------------------------------------------------->|
                                                                                          |->getPage()
********************/

auto HandleUpdateParams(AsyncWebServerRequest *aRequest) -> void
{
    const size_t paramsNr = aRequest->params();

    TLog::println("HandleUpdateParams. Params total: ");
    TLog::print(paramsNr);

    for (size_t i = 0; i < paramsNr; i++)
    {
        const AsyncWebParameter *const p = aRequest->getParam(i);
        const auto name = p->name();
        const auto value = p->value();

        TLog::println("Param [name, value] : [");
        TLog::print(name);
        TLog::print(", ");
        TLog::print(value);
        TLog::print("]");

        if (value.isEmpty())
            continue;

        if (name == "ip2geoKey")
            SaveIpGeolocation(value);

        if (name == "openWeatherKey")
            SaveOpenWeather(value);

        if (name == "wifiSsid")
            SaveWifiSSID(value);

        if (name == "wifiPassword")
            SaveWifiPassword(value);
    }

    SendWebPageResponse(aRequest);
}

auto SetupWiFiAccessPoint() -> void
{
    TLog::println("Setting AP (Access Point): ");
    const auto ssid = GetWifiSSID(SettingsType::factory);
    TLog::print(ssid);
    TLog::println("pass: ");
    const auto pass = GetWiFiPassword(SettingsType::factory);
    TLog::print(pass);
    WiFi.softAP(ssid, pass);

    TLog::println("AP IP address: ");
    TLog::print(WiFi.softAPIP());
}

auto LockingWiFiConnection() -> bool
{
    const auto wifiSSID = GetWifiSSID();
    const auto wifiPass = GetWiFiPassword();

    // Connect to Wi-Fi network with SSID and password
    constexpr uint32_t WIFI_RECON_DELAY_MILLISEC{10000};
    constexpr uint32_t WIFI_PROGRESS_DELAY_MILLISEC{500};

    auto status = wl_status_t::WL_IDLE_STATUS;
    for(int count{0}; count < 5 && wl_status_t::WL_CONNECTED != status; ++count)
    {
        TLog::println("Attempting to connect to SSID: ");
        TLog::print(wifiSSID);
        TLog::print(" ");

        status = WiFi.begin(wifiSSID, wifiPass);
        for(int split{0}
                ; wl_status_t::WL_CONNECTED != status && split < 10
                ; ++split, status = WiFi.status())
        {
            TLog::print(".");
            delay(WIFI_PROGRESS_DELAY_MILLISEC);
        }

        if(wl_status_t::WL_CONNECTED != status )
            delay(WIFI_RECON_DELAY_MILLISEC);
        else
            return true;
    }

    return false;
}

auto PrintWifiStatus() -> void
{
    // Print local IP address and start web server
    TLog::println("WiFi connected to SSID: ");
    TLog::print(WiFi.SSID());
    TLog::println("IP Address: ");
    TLog::print(WiFi.localIP());
    TLog::println("Signal strength (RSSI): ");
    TLog::print(WiFi.RSSI());
    TLog::print(" dBm");
    TLog::println();
}

// ===================================================
// Setup
// ===================================================
void setup()
{
    constexpr unsigned long SERIAL_MONITOR_SPEED{115200};
    Serial.begin(SERIAL_MONITOR_SPEED);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    TLog::println("=================================");

    pinMode(BUILDIN_LED_PIN, OUTPUT);

    RestoreStoredData();

    if (IsColdStart() || !LockingWiFiConnection())
        SetupWiFiAccessPoint();

    PrintWifiStatus();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/update", HTTP_GET, HandleUpdateParams);
    server.on("/restart", HTTP_GET, HandleDeviceSoftRestart);
    server.on("/favicon.ico", HTTP_GET, HandleFavIcon);
    server.onNotFound(handleNotFound);
    server.begin();
}

// ===================================================
// Loop
// ===================================================
void loop()
{
    // [ ]TODO: wrap into task or jobs
    // server.handleClient();
    // server.send(200, "text/html", getPage());
    // * must send data by websocket

    const auto newmil = millis();
    if (newmil - oldmil >= UPDATE_INTERVAL_MILLISEC)
    {
        digitalWrite(BUILDIN_LED_PIN, ledState);
        ledState = 1 - ledState;

        oldmil = newmil;
    }

    static bool once = false;
    if (newmil - oldmil2 >= 6 * UPDATE_INTERVAL_MILLISEC && false == once)
    {
        // Check WiFi connection status
        if (WL_CONNECTED == WiFi.status())
        {
            GetLocationCoordinates(GetIpGeoKey());
            GetForecast(GetOpenWeatherKey()
                        , String(Coordinates_.latitude)
                        , String(Coordinates_.longitude));
        }
        else
        {
            TLog::println("WiFi Disconnected");
        }
        oldmil2 = newmil;
        once = true;
    }
}

//===========================================================

/************************
get cooord
    v
has city name?
             |
    |------------------|
    v                  v
    no                yes
    v                  |
get city info by ip ---]----|
    v                  |    |
return coordinates <---|    |
    v                       |
get forcast by coordinate   |
    v                       |
print city/forecast info <--|
*************************/

/* Roadmap:

[x]TODO: get gps coordinates by ip
[x]TODO: get forecast by gps coordinates
[x]TODO: update wifi params from webpage
[x]TODO: update api keys params from webpage
[ ]TODO: visualizate forecast using google charts
[x]TODO: export to a new module RestoreStoredData
[ ]TODO: test connection and report before save them. Possible to lose wifi connection
[x]TODO: prebuild acton to gzip index.html
[x]TODO: soft restart button on web page
[ ]TODO: add status of coordinates acquisition
[ ]TODO: add status of forecast acquisition
[ ]TODO: in wifi AP mode web page freezes couse google charts js
[x]TODO: set default SSID and pass in case "no connection to router". now it is last
[x]TODO: setup and run default wifi access point on cold start

*/
