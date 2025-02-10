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
// #include "wifisecrets.h"
#include "stubs.h"
#include "resources.h"
#include <TLog.h>
#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"

// Pass our oneWire reference to Dallas Temperature.
// DallasTemperature sensors(&oneWire);
Dummy_DallasTemperature __sensors; // [ ]TODO: remove
// ---------------------------------------------------

constexpr int listenPort{80};
AsyncWebServer server(listenPort);

constexpr size_t arraySize{10};
int tempArray[arraySize] = {};
unsigned long timeArray[arraySize] = {};
unsigned long oldmil = 0UL;
unsigned long oldmil2 = 0UL;
const unsigned long UPDATE_INTERVAL_MILLISEC = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};
// ---------------------------------------------------

String processor(const String &aVar);
// ---------------------------------------------------

void SendWebPageResponse(AsyncWebServerRequest *aRequest)
{
    auto response = aRequest->beginResponse_P(200
                                            , "text/html"
                                            , index_html_gz_start
                                            , index_html_gz_size
                                            // , processor
                                            );
    response->addHeader("Content-Encoding", "gzip");
    aRequest->send(response);

    // aRequest->send_P(200, "text/html", index_html_template, processor);
}

// ==================================================
// Handle for page not found
// ==================================================
void handleNotFound(AsyncWebServerRequest *aRequest)
{
    SendWebPageResponse(aRequest);
}

// функция формирования содержимого WEB страницы
String processor(const String &aVar)
{
    TLog::print("processor aVar = ");
    TLog::println(aVar);

    if (aVar.equals("ARRAYPLACEHOLDER"))
    {
        String pointsStr;
        for (size_t i = 0; i < arraySize; ++i)
        {
            pointsStr += ",[";
            pointsStr += String(timeArray[i]);
            pointsStr += ",";
            pointsStr += String(tempArray[i]);
            pointsStr += "]";
        }
        TLog::println(pointsStr);
        return pointsStr;
    }

    return String();
}

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

auto HandleFavIcon(AsyncWebServerRequest *aRequest) -> void
{
    TLog::println("Requested favicon.ico");

    auto response = aRequest->beginResponse_P(200
                                            , "image/x-icon"
                                            , favicon_ico_gz_start
                                            , favicon_ico_gz_size
                                              // , processor
    );
    response->addHeader("Content-Encoding", "gzip");
    aRequest->send(response);
}

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

auto HandleDeviceSoftRestart(AsyncWebServerRequest *aRequest) -> void
{
    TLog::println("Handle Soft Restart");
    TLog::println();

    const char webpage[] PROGMEM = R"rawhtml(
        <!DOCTYPE HTML>
        <html>
        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0 maximum-scale=2.5, user-scalable=1">
            <title>Temperature Forecast Linechart demo</title>
        </head>
        <body>         
        <h2>Restarting...</h2>
        </body>
        </html>
    )rawhtml";
    aRequest->send(200, String("text/html"), webpage);

    ESP.restart();
}

// ===================================================
// Update values history
// ===================================================
auto AddNewMeasurement(int aNewVAlue) -> void
{
    constexpr size_t size{arraySize - 1};
    timeArray[size] = millis() / 1000UL;

    for (size_t i = 0; i < size; i++)
    {
        tempArray[i] = tempArray[i + 1];
        timeArray[i] = timeArray[i + 1];
    }

    tempArray[size] = aNewVAlue;
}

auto LockingWiFiConnection() -> void
{
    const auto wifiSSID = GetWifiSSID();
    // Connect to Wi-Fi network with SSID and password
    TLog::println("Connecting to ");
    TLog::print(wifiSSID);
    TLog::print(" ");

    WiFi.begin(wifiSSID, GetWiFiPassword());
    constexpr uint32_t WIFI_RECON_DELAY_MILLISEC{500};
    while (WL_CONNECTED != WiFi.status())
    {
        delay(WIFI_RECON_DELAY_MILLISEC);
        TLog::print(".");
    }

    // Print local IP address and start web server
    TLog::println("WiFi connected.");
    TLog::println("IP address: ");
    TLog::print(WiFi.localIP());
    TLog::println();
}

using callback__ = std::function<auto(const String &)->void>;      // [ ]FIXME:rename
auto TryToGetData(callback__ aFunc, const String &aApiKey) -> void // [x]FIXME:rename
{
    // Check WiFi connection status
    if (WL_CONNECTED == WiFi.status())
    {
        aFunc(aApiKey);
    }
    else
    {
        TLog::println("WiFi Disconnected");
    }
}

// ===================================================
// Setup
// ===================================================
void setup()
{
    constexpr unsigned long SERIAL_MONITOR_SPEED{115200};
    Serial.begin(SERIAL_MONITOR_SPEED);

    TLog::println("=================================");

    pinMode(BUILDIN_LED_PIN, OUTPUT);

    // [ ]TODO: setup and run default wifi access point on cold start
    RestoreStoredData();
    LockingWiFiConnection();

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
    // server.handleClient();
    // server.send(200, "text/html", getPage());
    // * must send data by websocket

    const auto newmil = millis();
    if (newmil - oldmil >= UPDATE_INTERVAL_MILLISEC)
    {
        int tempC = __sensors.getTempCByIndex(0U);

        TLog::println("The temperature in ticker is: ");
        TLog::print(tempC);
        TLog::print(" degrees C");

        digitalWrite(BUILDIN_LED_PIN, ledState);
        ledState = 1 - ledState;
        // [ ]TODO:need remove
        AddNewMeasurement(tempC);
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
get cooord -> has city name? ->yes------------------>|
                        |->no-->get city info by ip->|
                                         |           |->return coordinates-|
    |------------------------------------]---------------------------------|
    |                                    |
    v                                    |
get forcast by coordinate-->forecast-|   |
                                     |   v
                                     |-> print city/forecast info
*************************/

/* Roadmap:

[x]TODO: get gps coordinates by ip
[x]TODO: get forecast by gps coordinates
[x]TODO: update wifi params from webpage
[x]TODO: update api keys params from webpage
[ ]TODO: visualizate forecast using google charts
[x]TODO: export to a new module RestoreStoredData
[ ]TODO: test connection and report before save them. Possible to lose wifi connection
[ ]TODO: reset to default mode AP
[x]TODO: prebuild acton to gzip index.html
[x]TODO: soft restart button on web page
[ ]TODO: add status of coordinates acquisition
[ ]TODO: add status of forecast acquisition

*/
