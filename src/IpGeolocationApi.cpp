#include "IpGeolocationApi.h"

#include <HTTPClient.h>
#include <TLog.h>

auto GetLocationCoordinates(const String& aApiKey) -> void {// [ ]FIXME:rename
  const String serverName = "https://api.ipgeolocation.io/ipgeo";

  // const String serverPath = serverName + "?apiKey={api_key}";
  const String serverPath = serverName + "?apiKey=" + aApiKey;

  TLog::println(serverPath);

  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());

  // [ ]FIXME:remove
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) 
  {
    // [ ]TODO: handle response codes and JSONs 200 401 404
    TLog::println("HTTP Response code: ");
    TLog::print(httpResponseCode);
    const String payload = http.getString();
    TLog::println(payload);
  } 
  else 
  {
    TLog::println("Error code: ");
    TLog::print(httpResponseCode);
  }
  // Free resources
  http.end();
}