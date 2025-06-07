#include "WebSocketHandlers.h"

#include <ArduinoJson.h>
#include <functional>

#include "IpGeolocationApi.h"
#include "OpenWeatherApi.h"
#include "WifiAux.h"
#include "JsonAux.h"
#include "MessageDispatcher.h"

constexpr char TAG[] = "[WebSoc]";

namespace MessageType {
    const char IpGeolocationTest[]{"ip2geoTest"};
    const char OpenWeatherTest[]{"openWeatherTest"};
    const char FormFillRequest[]{"FormFill"};
    const char ChartDataRequest[]{"ChartDataRequest"};
    const char WiFiAPsRequest[]{"AcquireWiFiAPs"};
    const char RestartSystemRequest[]{"RestartSystem"};
    const char FactoryResetRequest[]{"ResetToDefaults"};
}

bool chartDataRequested{false};

using HandlerParams = std::tuple<AsyncWebSocket*, JsonDocument&&>;
MessageDispatcher<String, HandlerParams> dispatcher;

using ApiFunc = std::function<auto(const String&) -> bool>;
auto TestApiKey(const JsonDocument& aDoc, ApiFunc aFunc) -> bool
{
    const auto keyNode = aDoc["apikey"];
    if (!keyNode.is<String>())
        return false;

    const auto key = keyNode.as<String>();
    return aFunc(key);
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
    // Test if parsing succeeds.
    if (auto error = deserializeJson(doc, json); error)
    {
        log_w("%s JSON deserializition is failed. Error code: %s",TAG , error.f_str());
        return;
    }

    const auto msg = doc["message"];
    if (!msg.is<String>())
    {
        log_w("%s Unknown JSON format", TAG);
        return;
    }

    const auto msgType = msg.as<String>();
    log_d("%s Web socket message type is %s",TAG , msgType.c_str());

    dispatcher.Dispatch(msgType, std::make_tuple(aServer, std::move(doc)));
}

auto IpGeolocationTest(String aMsgType, HandlerParams aParams) -> void
{
    auto&& [server, doc] = aParams;
    const auto res = TestApiKey(doc, IpGeo::FetchData);
    log_d("%s IpGeolocation.io key check: %svalid", TAG, (res) ? "" : "In");

    const auto respond = SerializeRespondJSON(std::move(doc), aMsgType, res);
    server->textAll(respond);
}

auto OpenWeatherTest(String aMsgType, HandlerParams aParams) -> void
{
    auto&& [server, doc] = aParams;
    const auto res = TestApiKey(doc
                                , [](const String& aKey) -> bool
                                    {
                                        return OpenWeather::FetchData(aKey, String("0"), String("0"));
                                    });
    log_d("%s OpenWeathermap.org key check: %svalid", TAG, (res) ? "" : "In");

    const auto respond = SerializeRespondJSON(std::move(doc), aMsgType, res);
    server->textAll(respond);
}

auto FormFillRequest(String aMsgType, HandlerParams aParams) -> void
{
    auto&& [server, doc] = aParams;
    const String respond = SerializeFormStoredData(std::move(doc), "");
    log_d("%s Ready to send %s", TAG, respond.c_str());
    server->textAll(respond);
}

auto ChartDataRequest(String aMsgType, HandlerParams aParams) -> void
{
    log_d("%s Update chart weather data", TAG);

    chartDataRequested = true;
}

auto WiFiAPsRequest(String aMsgType, HandlerParams aParams) -> void
{
    log_i("%s Start WiFi network scan", TAG);
    // [ ]TODO: core1 WDT restart device here.
    // proposal cannont scan in wifi client mode
    StartWiFiScanAsync(WiFi);
}

auto RestartSystem(String aMsgType, HandlerParams aParams) -> void
{
    // [ ]TODO: make a restart
    log_i("%s Restart system", TAG);
    ESP.restart();
}

auto FactoryResert(String aMsgType, HandlerParams aParams) -> void
{
    // [ ]TODO: make a reset. Still do not know what to reset
    log_i("%s Reset to defaults", TAG);
}

auto UnknownMessage(String aMsgType, HandlerParams aParams) -> void
{
    log_w("%s Unknown message", TAG);
}

auto InitializeWebSocketMessageDispatching() -> void
{
    dispatcher.RegisterHandler(MessageType::IpGeolocationTest, IpGeolocationTest);
    dispatcher.RegisterHandler(MessageType::OpenWeatherTest, OpenWeatherTest);
    dispatcher.RegisterHandler(MessageType::FormFillRequest, FormFillRequest);
    dispatcher.RegisterHandler(MessageType::ChartDataRequest, ChartDataRequest);
    dispatcher.RegisterHandler(MessageType::WiFiAPsRequest, WiFiAPsRequest);
    dispatcher.RegisterHandler(MessageType::RestartSystemRequest, RestartSystem);
    dispatcher.RegisterHandler(MessageType::FactoryResetRequest, FactoryResert);
    dispatcher.RegisterUnknownMessageHandler(UnknownMessage);
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