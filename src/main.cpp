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

#include "NvsPreferences.h"
#include "resources.h"
#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "WebServerHandlers.h"

unsigned long oldmil = 0UL;
unsigned long oldmil2 = 0UL;
const unsigned long UPDATE_INTERVAL_MILLISEC = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};

AsyncWebSocket websocket("/ws");
// ---------------------------------------------------


auto wifiscan() -> String;


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
    const auto ssid = GetWifiSSID(SettingsType::factory);
    const auto pass = GetWiFiPassword(SettingsType::factory);
    log_i("Setting default AP (Access Point) ssid: %s, pass: %s", ssid, pass);
    WiFi.mode(wifi_mode_t::WIFI_MODE_AP);
    WiFi.disconnect();
    wifiscan();
    WiFi.softAP(ssid, pass);

    const auto ip = WiFi.softAPIP();
    log_i("AP IP address: %s", ip.toString().c_str());
}

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

auto PrintWiFiAPPrettyTable(const int16_t aTotal) -> void
{
    log_i("Nr | SSID                             | RSSI | CH | Encryption");
    for (auto i = 0; i < aTotal; ++i)
    {
        // Print SSID and RSSI for each network found
        log_i("%2d | %-32.32s | %4d | %2d | %s", i + 1
                , WiFi.SSID(i).c_str()
                , WiFi.RSSI(i)
                , WiFi.channel(i)
                , to_string(WiFi.encryptionType(i)).c_str()
            );
    }
}

auto WiFiAPtoJSON(const int16_t aTotal) -> String
{
    JsonDocument doc;
    doc["message"] = "wifiAps";

    for (auto i = 0; i < aTotal; ++i)
    {
        doc["APs"][i]["ssid"] = WiFi.SSID(i).c_str();
        doc["APs"][i]["rssi"] = WiFi.RSSI(i);
        doc["APs"][i]["channel"] = WiFi.channel(i);
        doc["APs"][i]["encryption"] = to_string(WiFi.encryptionType(i)).c_str();
    }

    String serial;
    serializeJson(doc, serial);
    log_d("Available WiFi APs response json %s", serial.c_str());

    return serial;
}

auto wifiscan() -> String
{
    // WiFi.scanNetworks will return the number of networks found.
    const auto n = WiFi.scanNetworks();
    log_i("Scan done");
    if (0 == n)
        log_i("No networks found");
    else
    {
        log_i("%d networks found", n);
        PrintWiFiAPPrettyTable(n);
        const auto json = WiFiAPtoJSON(n);
        // [x]TODO: wrap into JSON

        // Delete the scan result to free memory for code below.
        WiFi.scanDelete();

        return json;
    }

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    return {};
}

auto LockingWiFiConnection() -> bool
{
    const auto wifiSSID = GetWifiSSID();
    const auto wifiPass = GetWiFiPassword();

    // Connect to Wi-Fi network with SSID and password
    constexpr uint32_t WIFI_RECON_DELAY_MILLISEC{10000};
    constexpr uint32_t WIFI_PROGRESS_DELAY_MILLISEC{500};

    auto status = wl_status_t::WL_IDLE_STATUS;
    for(int count{0}; count < 2 && wl_status_t::WL_CONNECTED != status; ++count)
    {
        log_i("Attempting to connect to SSID: %s", wifiSSID.c_str());

        status = WiFi.begin(wifiSSID, wifiPass);
        for(int split{0}
                ; wl_status_t::WL_CONNECTED != status && split < 10
                ; ++split, status = WiFi.status())
        {
            log_i("Waiting");
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
    log_i("WiFi status:\r\nConnected to SSID: %s \r\nIP Address: %s\r\nSignal strength (RSSI): %d dBm"
            , WiFi.SSID().c_str()
            , WiFi.localIP().toString().c_str()
            , WiFi.RSSI()
        );
}

auto SerializeFormStoredData(JsonDocument&& aDoc, const String& aMsgType, bool aValidated) -> String
{
    // [x]TODO: rename and implement
    // [ ]TODO: need interface refactoring

    // drop request json
    aDoc.clear();

    aDoc.add("message");
    aDoc.add("FormFillStoredData");
    aDoc.add("WifiSsid");
    aDoc.add(GetWifiSSID());
    aDoc.add("WiFiPassword");
    // [ ]TODO: need to think about this
    // aDoc.add(GetWiFiPassword());
    aDoc.add("");
    aDoc.add("Longitude");
    aDoc.add("[ ]TODO: longitude");
    aDoc.add("Latitude");
    aDoc.add("[ ]TODO: latitude");
    aDoc.add("IpGeolocKey");
    aDoc.add(GetIpGeoKey());
    aDoc.add("OpenWeatherKey");
    aDoc.add(GetOpenWeatherKey());

    String serial;
    serializeJson(aDoc, serial);
    log_d("Check response json %s", serial.c_str());

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

    if (doc["apikey"].is<String>())
    {
        const auto key = doc["apikey"].as<String>();
        // [ ]TODO: split function
        if (String("ip2geoTest") == msgType)
        {
            const auto res = GetLocationCoordinates(key);
            log_d("IpToGeo key check is %s ", (res) ? "Ok": "failed");

            const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
            websocket.textAll(respond.c_str());
        }
        else if (String("openWeatherTest") == msgType)
        {
            const auto res = GetForecast(key, String("0"), String("0"));
            log_d("OpenWeather key check is %s ", (res) ? "Ok": "failed");

            const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
            websocket.textAll(respond.c_str());
        }
        else
            log_w("Unknown message");
    }
    else if (String("FormFill") == msgType)
    {
        // [x]TODO: update form with stored data
        const String respond = SerializeFormStoredData(std::move(doc), "", "");
        log_d("Ready to send %s", respond.c_str());
        websocket.textAll(respond.c_str());
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

    if (IsColdStart() || !LockingWiFiConnection())
        SetupWiFiAccessPoint();

    PrintWifiStatus();

    wifiscan();

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
            GetLocationCoordinates(GetIpGeoKey());
            GetForecast(GetOpenWeatherKey()
                        , String(Coordinates_.latitude)
                        , String(Coordinates_.longitude));
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
