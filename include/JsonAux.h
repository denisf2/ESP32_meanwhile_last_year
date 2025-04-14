#ifndef JSONAUX_H__
#define JSONAUX_H__

#include <ArduinoJson.h>
#include <WiFi.h>

auto WiFiAPtoJSON(WiFiClass& aWiFi, const int16_t aTotal) -> String;
auto SerializeRespondJSON(JsonDocument&& aDoc, const String& aMsgType, bool aValidated) -> String;
auto SerializeFormStoredData(JsonDocument&& aDoc, const String& aMsgType) -> String;

#endif /*JSONAUX_H__*/