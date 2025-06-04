#ifndef WEBSOCKETHANDLERS_H__
#define WEBSOCKETHANDLERS_H__

#include <ESPAsyncWebServer.h>

auto ProcessWSData(AsyncWebSocket * aServer, const AwsFrameInfo* const aFrameInfo, const uint8_t* const aData) -> void;
auto OnEvent(AsyncWebSocket *aServer, AsyncWebSocketClient *aClient, AwsEventType aType, void *aArg, uint8_t *aData, size_t aLen) -> void;
auto InitializeWebSocketMessageDispatching() -> void;

extern bool chartDataRequested;

#endif /* WEBSOCKETHANDLERS_H__ */