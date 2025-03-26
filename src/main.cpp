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
#include <ArduinoJson.h>

#include <functional>
#include <string>
#include <exception>

#include "NvsPreferences.h"
#include "resources.h"
#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "WebServerHandlers.h"
#include "WifiAux.h"
#include "JsonAux.h"

unsigned long oldmil = 0UL;
unsigned long oldmil2 = 0UL;
constexpr unsigned long UPDATE_INTERVAL_MILLISEC = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};

AsyncWebSocket websocket("/ws");
// ---------------------------------------------------

// ==================================================
// Handle submit form
// ==================================================
void handleSubmit(AsyncWebServerRequest *aRequest)
{
    // [ ]TODO: do something with button1
    if (aRequest->hasArg("button1"))
    {
        log_i("button1 was pressed");
    }

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

    log_i("Params total: %d", paramsNr);

    for (size_t i = 0; i < paramsNr; i++)
    {
        const AsyncWebParameter *const p = aRequest->getParam(i);
        const auto name = p->name();
        const auto value = p->value();

        log_i("Param [name, value] : [%s, %s]", name.c_str(), value.c_str());

        if (value.isEmpty())
            continue;

        if (name.equals("ip2geoKey"))
            SaveIpGeolocation(value);

        if (name.equals("openWeatherKey"))
            SaveOpenWeather(value);

        if (name.equals("latitude"))
        {
            SaveLatitude(value);
            SaveAutoLocation(false);
        }

        if (name.equals("longitude"))
        {
            SaveLongitude(value);
            SaveAutoLocation(false);
        }

        if (name.equals("wifiSsid"))
            SaveWifiSSID(value);

        if (name.equals("wifiPassword"))
            SaveWifiPassword(value);
    }

    // [ ]TODO: why do we need send in response whole page?
    SendWebPageResponse(aRequest);
}

auto ProcessWSData(const AwsFrameInfo * const aFrameInfo, const uint8_t * const aData) -> void
{
    // taking care only JSON
    if (AwsFrameType::WS_TEXT != aFrameInfo->opcode)
    {
        log_i("Do not care binary formats");
        return;
    }

    String json(reinterpret_cast<const char* const >(aData));
    log_d("Incoming JSON %s", json.c_str());

    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, json);

    // Test if parsing succeeds.
    if (error)
    {
        log_w("JSON deserializition is failed. Error code: %s", error.f_str());
        return;
    }

    // [ ]TODO: need refactoring
    // messages dispatching
    if (!doc["message"].is<String>())
    {
        log_w("Unknown JSON format");
        return;
    }

    const auto msgType = doc["message"].as<String>();

    if (msgType.equals("ip2geoTest"))
    {
        if (doc["apikey"].is<String>())
        {
            const auto key = doc["apikey"].as<String>();
            // [ ]TODO: split function
            const auto res = GetLocationCoordinates(key);
            log_d("IpToGeo key check is %s ", (res) ? "Ok": "failed");

            const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
            websocket.textAll(respond.c_str());
        }
    }
    else if (msgType.equals("openWeatherTest"))
    {
        if (doc["apikey"].is<String>())
        {
            const auto key = doc["apikey"].as<String>();
            // [ ]TODO: split function
            const auto res = GetForecast(key, String("0"), String("0"));
            log_d("OpenWeather key check is %s ", (res) ? "Ok": "failed");

            const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
            websocket.textAll(respond.c_str());
        }
    }
    else if (msgType.equals("openWeatherTest"))
    {
        // [x]TODO: update form with stored data
        const String respond = SerializeFormStoredData(std::move(doc), "", "");
        log_d("Ready to send %s", respond.c_str());
        websocket.textAll(respond.c_str());

        //[ ]TODO: remove from here after WDT fix
        websocket.textAll(ScanWiFiAPsJSON(WiFi).c_str());
    }
    else if (msgType.equals("AcquireWiFiAPs"))
    {
        log_d("nothing to do");
        // [ ]TODO: core1 WDT restart device here.
        // proposal cannont scan in wifi client mode
        // websocket.textAll(ScanWiFiAPsJSON(WiFi).c_str());
    }
    else
        log_w("Unknown message");
}

auto OnEvent(AsyncWebSocket *aServer, AsyncWebSocketClient *aClient, AwsEventType aType, void *aArg, uint8_t *aData, size_t aLen) -> void
{
    switch (aType)
    {
        case AwsEventType::WS_EVT_CONNECT:
            log_i("WebSocket[%s][%u] connected from %s"
                    , aServer->url()
                    , aClient->id()
                    , aClient->remoteIP().toString().c_str());
            aClient->ping();
        break;

        case AwsEventType::WS_EVT_DISCONNECT:
        log_i("WebSocket[%s][%u] disconnect: %s"
                    , aServer->url()
                    , aClient->id()
                    , aClient->remoteIP().toString().c_str());
        break;

        case AwsEventType::WS_EVT_DATA:
        {
            auto info = reinterpret_cast<const AwsFrameInfo * const>(aArg);
            log_d("WebSocket[%s][%u] %s-message[%llu]: "
                , aServer->url()
                , aClient->id()
                , (AwsFrameType::WS_TEXT == info->opcode) ? "text" : "binary"
                , info->len);
            // [ ] TODO: what if multiframe data?
            ProcessWSData(info, aData);
            break;
        }

        case AwsEventType::WS_EVT_PONG:
            log_i("WebSocket[%s][%u] pong[%u]: %s"
                    , aServer->url()
                    , aClient->id()
                    , aLen
                    , (0 < aLen) ? reinterpret_cast<char*>(aData) : "");
                    break;

        case AwsEventType::WS_EVT_ERROR:
            log_e("WebSocket[%s][%u] error(%u): %s"
                    , aServer->url()
                    , aClient->id()
                    , *(static_cast<uint16_t*>(aArg))
                    , reinterpret_cast<char*>(aData));
        break;

        default:
            break;
    }
}

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

auto acquire_coordinates_rename_me() -> void
{
    // [ ]TODO: chose user/automatic coordinates
    if (GetAutoLocation())
    {
        // case automatic
        GetLocationCoordinates(GetIpGeoKey());
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
            log_w("%s %s", msg, aExc.what());
        }
        catch (const std::out_of_range &aExc)
        {
            log_w("%s %s", msg, aExc.what());
        }
        catch (const std::exception &aExc)
        {
            log_w("%s %s", msg, aExc.what());
        }
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
            acquire_coordinates_rename_me();
            GetForecast(GetOpenWeatherKey()
                        , String(coordinates.latitude)
                        , String(coordinates.longitude));
        }
        else
        {
            log_i("WiFi Disconnected");
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
