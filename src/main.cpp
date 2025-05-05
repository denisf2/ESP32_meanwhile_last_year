// ======================================================
// Webserver program that gets own coordinates by IP address
// and acquiring weather forecast info for today and
// historical weather for several days back and before today.

// Using services:
// https://ipgeolocation.io/
// https://openweathermap.org/
// https://open-meteo.com/

// https://ipgeolocation.io/ip-location-api.html
// https://openweathermap.org/api
// https://openweathermap.org/history
// https://openweathermap.org/api/geocoding-api
// https://open-meteo.com/en/docs
// ======================================================

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <esp_sntp.h>

#include <functional>
#include <string>
#include <exception>

#include "NvsPreferences.h"
#include "resources.h"
#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "OpenMeteoApi.h"
#include "WebServerHandlers.h"
#include "WebSocketHandlers.h"
#include "WifiAux.h"
#include "timeAux.h"
#include "JsonAux.h"

// [ ]TODO: need refactoring to prev time stamps
unsigned long oldmil1 = 0UL;
unsigned long oldmil2 = 0UL;
unsigned long oldmil3 = 0UL;
unsigned long oldmil4 = 0UL;
unsigned long oldmil5 = 0UL;

constexpr unsigned long UPDATE_INTERVAL_10_S = 10000UL;
constexpr unsigned long UPDATE_INTERVAL_1_S = 1000UL;
constexpr unsigned long UPDATE_INTERVAL_05_S = 500UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};

AsyncWebSocket websocket("/ws");

// ---------------------------------------------------

auto InitWebSocket() -> void
{
    websocket.onEvent(OnEvent);
    server.addHandler(&websocket);
}

auto InitWebServer() -> void
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/update", HTTP_GET, HandleUpdateParams);
    server.on("/restart", HTTP_GET, HandleDeviceSoftRestart);
    server.on("/favicon.ico", HTTP_GET, HandleFavIcon);
    server.onNotFound(handleNotFound);
    server.begin();
}

auto PreWiFiSNTPInit() -> void
{
    /**
     * NTP server address could be acquired via DHCP,
     *
     * NOTE: This call should be made BEFORE esp32 acquires IP address via DHCP,
     * otherwise SNTP option 42 would be rejected by default.
     * NOTE: configTime() function call if made AFTER DHCP-client run
     * will OVERRIDE acquired NTP server address
     */
    esp_sntp_servermode_dhcp(true);  // (optional)
}

auto InitNTPClient() -> void
{
    // ntpTimeClient.begin();
    // ntpTimeClient.setTimeOffset(0); // UTC time

    // =================
    // set notification call-back function
    sntp_set_time_sync_notification_cb(timeavailable);

    /**
     * This will set configured ntp servers and constant TimeZone/daylightOffset
     * should be OK if your time zone does not need to adjust daylightOffset twice a year,
     * in such a case time adjustment won't be handled automagically.
     */
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

    /**
     * A more convenient approach to handle TimeZones with daylightOffset
     * would be to specify a environment variable with TimeZone definition including daylight adjustmnet rules.
     * A list of rules for your zone could be obtained from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
     */
    //configTzTime(time_zone, ntpServer1, ntpServer2);
}

auto job_acquire_coordinates(unsigned long aCurrent) -> void
{
    // [ ]TODO: make run at start and once per hour
    static bool once = false;
    if (aCurrent - oldmil4 >= 4 * UPDATE_INTERVAL_10_S && false == once)
    {
        // [ ]TODO: choose user stored/automatic coordinates
        // if (GetAutoLocation())
        if (true)
        {
            // case automatic
            GetLocationCoordinates(GetIpGeoKey());

            SaveLatitude(std::to_string(coordinates.latitude).c_str());
            SaveLongitude(std::to_string(coordinates.longitude).c_str());
        }
        else
        {
            constexpr char msg[] = "coordinate update failed";
            // case manual
            try
            {
                Coordinates_t tmp{coordinates};

                // use std::stod to catch exceptions
                // String::toDouble() does not provide this functionality
                tmp.latitude = std::stod(GetLatitude().c_str());
                tmp.longitude = std::stod(GetLongitude().c_str());

                coordinates = tmp;
            }
            catch (const std::invalid_argument &aExc)
            {
                log_e("%s. Invalid argument: %s", msg, aExc.what());
            }
            catch (const std::out_of_range &aExc)
            {
                log_e("%s. Out of range: %s", msg, aExc.what());
            }
            catch (const std::exception &aExc)
            {
                log_e("%s. Other exception: %s", msg, aExc.what());
            }
        }

        oldmil4 = aCurrent;
        once = true;
    }
}

auto job_working_led_blink(unsigned long aCurrent) -> void
{
    if (aCurrent - oldmil1 >= UPDATE_INTERVAL_10_S)
    {
        digitalWrite(BUILDIN_LED_PIN, ledState);
        ledState = 1 - ledState;

        oldmil1 = aCurrent;
    }
}

auto job_request_weather_data(unsigned long aCurrent) -> void
{
    // [ ]TODO: make run at start and once per hour
    static bool once = false;
    if (aCurrent - oldmil2 >= 6 * UPDATE_INTERVAL_10_S && false == once)
    {
        // Check WiFi connection status
        if (WL_CONNECTED == WiFi.status())
        {
            // last year
            GetWeatherLastYear(String(coordinates.latitude)
                            , String(coordinates.longitude));
            // last three days
            GetWeatherLastWeek(String(coordinates.latitude)
                            , String(coordinates.longitude));

            GetForecast(GetOpenWeatherKey()
                            , String(coordinates.latitude)
                            , String(coordinates.longitude));
        }
        else
        {
            log_i("WiFi Disconnected");
        }
        oldmil2 = aCurrent;
        once = true;
    }
}

auto job_update_chart_data(unsigned long aCurrent) -> void
{
    // [ ]TODO: add some delay
    if(chartDataRequested && chartHistoryDataReady)
    {
        auto respond = GetChartData();
        websocket.textAll(respond);

        chartDataRequested = false;
    }
}

auto job_check_wifi_scan(unsigned long aCurrent) -> auto
{
    if (!scanInProgress)
        return;

    if (aCurrent - oldmil5 >= UPDATE_INTERVAL_1_S)
    {
        // [x]TODO: add some delay
        auto res = CheckWiFiScan(WiFi);
        if (res.has_value())
        {
            websocket.textAll(res.value());
            log_d("WiFi scan is ready");
        }

        oldmil5 = aCurrent;
    }
}

auto InitSerialMonitor() -> void
{
    constexpr unsigned long SERIAL_MONITOR_SPEED{115200};
    Serial.begin(SERIAL_MONITOR_SPEED);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }
}

auto InitBlinkingLED() -> void
{
    pinMode(BUILDIN_LED_PIN, OUTPUT);
}

// ===================================================
// Setup
// ===================================================
void setup()
{
    InitSerialMonitor();
    InitBlinkingLED();

    RestoreStoredData();

    // PreWiFiSNTPInit();

    if (IsColdStart() || !LockingWiFiConnection(WiFi))
        SetupWiFiAccessPoint(WiFi);

    PrintWifiStatus(WiFi);

    InitNTPClient();

    InitWebSocket();
    InitWebServer();
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

    websocket.cleanupClients();

    const auto newmil = millis();

    job_working_led_blink(newmil);
    job_acquire_coordinates(newmil);
    job_request_weather_data(newmil);
    job_update_chart_data(newmil);
    job_check_wifi_scan(newmil);
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

// show user a page with plot
// current temperature
// meanwhile this day last year min/max temp per day
// last 3 days and 3 days before today current temp and min/max for last year
// plot graph current temp as line min/max as bar