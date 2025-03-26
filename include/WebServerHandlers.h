#ifndef WEBSERVERHANDLERS_H_
#define WEBSERVERHANDLERS_H_

#include <ESPAsyncWebServer.h>

auto HandleFavIcon(AsyncWebServerRequest *aRequest) -> void;
auto HandleDeviceSoftRestart(AsyncWebServerRequest *aRequest) -> void;
auto SendWebPageResponse(AsyncWebServerRequest *aRequest) -> void;
auto handleNotFound(AsyncWebServerRequest *aRequest) -> void;
auto HandleUpdateParams(AsyncWebServerRequest *aRequest) -> void;
auto handleRoot(AsyncWebServerRequest *aRequest) -> void;

extern AsyncWebServer server;
#endif /* WEBSERVERHANDLERS_H_ */