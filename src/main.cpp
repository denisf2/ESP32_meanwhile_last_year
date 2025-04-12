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
#include <NTPClient.h>
#include <WiFiUdp.h>

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

// [ ]TODO: need refactoring to prev time stamps
unsigned long oldmil1 = 0UL;
unsigned long oldmil2 = 0UL;
unsigned long oldmil3 = 0UL;
unsigned long oldmil4 = 0UL;

constexpr unsigned long UPDATE_INTERVAL_MILLISEC = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};

AsyncWebSocket websocket("/ws");

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
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

auto InitNTPClient() -> void
{
    timeClient.begin();
    timeClient.setTimeOffset(0); // UTC time
}

auto job_acquire_coordinates(unsigned long aCurrent) -> void
{
    // [ ]TODO: make run at start and once per hour
    static bool once = false;
    if (aCurrent - oldmil4 >= 4 * UPDATE_INTERVAL_MILLISEC && false == once)
    {
        // [ ]TODO: chose user/automatic coordinates
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
    if (aCurrent - oldmil1 >= UPDATE_INTERVAL_MILLISEC)
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
    if (aCurrent - oldmil2 >= 6 * UPDATE_INTERVAL_MILLISEC && false == once)
    {
        // Check WiFi connection status
        if (WL_CONNECTED == WiFi.status())
        {
            GetWeatherHistory(String(coordinates.latitude)
                            , String(coordinates.longitude)
                            , timeClient.getEpochTime());
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

auto job_update_time(unsigned long aCurrent) -> void
{
    static bool once = false;
    if (aCurrent - oldmil3 >= 6 * UPDATE_INTERVAL_MILLISEC && false == once)
    {
        timeClient.update();

        oldmil3 = aCurrent;
        once = true;
    }
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

    pinMode(BUILDIN_LED_PIN, OUTPUT);

    RestoreStoredData();

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
    job_update_time(newmil);
    job_request_weather_data(newmil);
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