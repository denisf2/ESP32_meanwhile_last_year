#include "WebSocketHandlers.h"

#include <ArduinoJson.h>

#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "WifiAux.h"
#include "JsonAux.h"

auto ProcessWSData(AsyncWebSocket * aServer, const AwsFrameInfo* const aFrameInfo, const uint8_t* const aData) -> void
{
    // taking care only JSON
    if (AwsFrameType::WS_TEXT != aFrameInfo->opcode)
    {
        log_i("Do not care binary formats");
        return;
    }

    String json(reinterpret_cast<const char* const>(aData), aFrameInfo->len);
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

    // [ ]TODO: messages dispatching needs refactoring
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
            log_d("IpToGeo key check is %s ", (res) ? "Ok" : "failed");

            const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
            aServer->textAll(respond);
        }
    }
    else if (msgType.equals("openWeatherTest"))
    {
        if (doc["apikey"].is<String>())
        {
            const auto key = doc["apikey"].as<String>();
            // [ ]TODO: split function
            const auto res = GetForecast(key, String("0"), String("0"));
            log_d("OpenWeather key check is %s ", (res) ? "Ok" : "failed");

            const auto respond = SerializeRespondJSON(std::move(doc), msgType, res);
            aServer->textAll(respond);
        }
    }
    else if (msgType.equals("FormFill"))
    {
        // [x]TODO: update form with stored data
        const String respond = SerializeFormStoredData(std::move(doc), "");
        log_d("Ready to send %s", respond.c_str());
        aServer->textAll(respond);

        //[ ]TODO: remove from here after WDT fix
        aServer->textAll(ScanWiFiAPsJSON(WiFi));
    }
    else if (msgType.equals("AcquireWiFiAPs"))
    {
        log_d("nothing to do");
        // [ ]TODO: core1 WDT restart device here.
        // proposal cannont scan in wifi client mode
        // aServer->textAll(ScanWiFiAPsJSON(WiFi));
    }
    else
        log_w("Unknown message");
}

auto OnEvent(AsyncWebSocket *aServer, AsyncWebSocketClient *aClient, AwsEventType aType, void *aArg, uint8_t *aData, size_t aLen) -> void
{
    switch (aType)
    {
        case AwsEventType::WS_EVT_CONNECT:
        {
            log_i("WebSocket[%s][%u] connected from %s"
                , aServer->url()
                , aClient->id()
                , aClient->remoteIP().toString().c_str());

            aClient->ping();
            break;
        }

        case AwsEventType::WS_EVT_DISCONNECT:
        {
            log_i("WebSocket[%s][%u] disconnect: %s"
                , aServer->url()
                , aClient->id()
                , aClient->remoteIP().toString().c_str());

            break;
        }

        case AwsEventType::WS_EVT_DATA:
        {
            auto info = reinterpret_cast<const AwsFrameInfo * const>(aArg);
            log_d("WebSocket[%s][%u] %s-message[%llu]: "
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
            log_i("WebSocket[%s][%u] pong[%u]: %s"
                , aServer->url()
                , aClient->id()
                , aLen
                , (0 < aLen) ? reinterpret_cast<char*>(aData) : "");

            break;
        }

        case AwsEventType::WS_EVT_ERROR:
        {
            log_e("WebSocket[%s][%u] error(%u): %s"
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