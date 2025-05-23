#include "WebSocketHandlers.h"

#include <ArduinoJson.h>

#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "WifiAux.h"
#include "JsonAux.h"

const char TAG[] = "[WebSoc]";

bool chartDataRequested{false};

auto TestIpGeolocationKey(const JsonDocument& aDoc) -> bool
{
    const auto keyNode = aDoc["apikey"];
    if (!keyNode.is<String>())
        return false;

    const auto key = keyNode.as<String>();
    return GetLocationCoordinates(key);
}

auto TestOpenweatherKey(const JsonDocument& aDoc) -> bool
{
    const auto keyNode = aDoc["apikey"];
    if (!keyNode.is<String>())
        return false;

    const auto key = keyNode.as<String>();
    return GetForecast(key, String("0"), String("0"));
}

auto getTestResponse() -> String
{
    return String("{\"message\" : \"ChartDataResponse\",\
\"current\":[14.3,12.2,15.1,14.8],\
\"daily\":{\
\"time\":[\
\"2024-04-08\",\"2024-04-09\",\"2024-04-10\",\"2024-04-11\",\"2024-04-12\",\"2024-04-13\",\"2024-04-14\"],\
\"temperature_2m_max\":[16.4,18.4,19.2,18.3,14.3,12.3,12.6],\
\"temperature_2m_min\":[7.0,8.2,9.8,7.8,6.3,2.9,4.0]}}"
    );
}

// --------------------------------------------------------
// Dispatching web application level messages
auto ProcessWSData(AsyncWebSocket * aServer, const AwsFrameInfo* const aFrameInfo, const uint8_t* const aData) -> void
{
    // taking care only JSON
    if (AwsFrameType::WS_TEXT != aFrameInfo->opcode)
    {
        log_i("%s Do not care binary formats", TAG);
        return;
    }

    String json(reinterpret_cast<const char* const>(aData), aFrameInfo->len);
    log_d("%s Incoming JSON %s", TAG, json.c_str());

    // Allocate the JSON document
    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, json);

    // Test if parsing succeeds.
    if (error)
    {
        log_w("%s JSON deserializition is failed. Error code: %s",TAG , error.f_str());
        return;
    }

    // [ ]TODO: messages dispatching needs refactoring
    const auto msg = doc["message"];
    if (!msg.is<String>())
    {
        log_w("%s Unknown JSON format", TAG);
        return;
    }

    const auto msgType = msg.as<String>();
    log_d("%s Web socket message type is %s",TAG , msgType.c_str());

    if (msgType.equals("ip2geoTest"))
    {
        const auto res = TestIpGeolocationKey(doc);
        log_d("%s IpGeolocation.io key check: %svalid", TAG, (res) ? "" : "In");

        const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
        aServer->textAll(respond);
    }
    else if (msgType.equals("openWeatherTest"))
    {
        const auto res = TestOpenweatherKey(doc);
        log_d("%s OpenWeathermap.org key check: %svalid", TAG, (res) ? "" : "In");

        const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
        aServer->textAll(respond);
    }
    else if (msgType.equals("FormFill"))
    {
        // [x]TODO: update form with stored data
        const String respond = SerializeFormStoredData(std::move(doc), "");
        log_d("%s Ready to send %s", TAG, respond.c_str());
        aServer->textAll(respond);
    }
    else if (msgType.equals("ChartDataRequest"))
    {
        log_d("%s Update chart weather data", TAG);

        chartDataRequested = true;
    }
    else if (msgType.equals("AcquireWiFiAPs"))
    {
        log_i("%s Start WiFi network scan", TAG);
        // [ ]TODO: core1 WDT restart device here.
        // proposal cannont scan in wifi client mode
        StartWiFiScanAsync(WiFi);
    }
    else if (msgType.equals("RestartSystem"))
    {
        // [ ]TODO: make a restart
        log_i("%s Restart system", TAG);
        ESP.restart();
    }
    else if (msgType.equals("ResetToDefaults"))
    {
        // [ ]TODO: make a reset. Still do not know what to reset
        log_i("%s Reset to defaults", TAG);
    }
    else
        log_w("%s Unknown message", TAG);
}

// --------------------------------------------------------
// Dispatching web socket level message types
auto OnEvent(AsyncWebSocket *aServer, AsyncWebSocketClient *aClient, AwsEventType aType, void *aArg, uint8_t *aData, size_t aLen) -> void
{
    switch (aType)
    {
        case AwsEventType::WS_EVT_CONNECT:
        {
            log_i("%s WebSocket[%s][%u] connected from %s"
                , TAG
                , aServer->url()
                , aClient->id()
                , aClient->remoteIP().toString().c_str());

            aClient->ping();
            break;
        }

        case AwsEventType::WS_EVT_DISCONNECT:
        {
            log_i("%s WebSocket[%s][%u] disconnect: %s"
                , TAG
                , aServer->url()
                , aClient->id()
                , aClient->remoteIP().toString().c_str());

            break;
        }

        case AwsEventType::WS_EVT_DATA:
        {
            auto info = reinterpret_cast<const AwsFrameInfo * const>(aArg);
            log_d("%s WebSocket[%s][%u] %s-message[%llu]: "
                , TAG
                , aServer->url()
                , aClient->id()
                , (AwsFrameType::WS_TEXT == info->opcode) ? "text" : "binary"
                , info->len);

            // [ ] TODO: what if multiframe data?
            ProcessWSData(aServer, info, aData);
            break;
        }

        case AwsEventType::WS_EVT_PONG:
        {
            log_i("%s WebSocket[%s][%u] pong[%u]: %s"
                , TAG
                , aServer->url()
                , aClient->id()
                , aLen
                , (0 < aLen) ? reinterpret_cast<char*>(aData) : "");

            break;
        }

        case AwsEventType::WS_EVT_ERROR:
        {
            log_e("%s WebSocket[%s][%u] error(%u): %s"
                , TAG
                , aServer->url()
                , aClient->id()
                , *(static_cast<uint16_t*>(aArg))
                , reinterpret_cast<char*>(aData));

            break;
        }

        default:
            break;
    }
}