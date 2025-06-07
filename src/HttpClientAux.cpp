#include "HttpClientAux.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

constexpr char TAG[] = "[HttpClientAux]";

auto SendGetRequest(const String& aUrl) -> std::optional<String>
{
    WiFiClientSecure client;
    // Skip certificate verification only in dev
    client.setInsecure();

    HTTPClient https;
    https.useHTTP10(true);
    // Your Domain name with URL path or IP address with path
    https.begin(client, aUrl);

    // const char* apiKeyHeader = "x-api-key";  // or "Authorization"
    // const char* apiKeyValue = "your_actual_api_key";

    // Add API key to headers
    // https.addHeader(apiKeyHeader, aApiKey);
    // https.addHeader("Content-Type", "application/json");

    // [ ]FIXME:remove
    // If you need server authentication, insert user and password below
    // https.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    // Send HTTP GET request
    int httpResponseCode = https.GET();
    if (httpResponseCode <= 0)
    {
        log_w("%s Error code: %d", TAG, httpResponseCode);
        // Free resources
        https.end();

        return std::nullopt;
    }

    // [ ]TODO: handle response codes and JSONs 200 400 401 404
    log_d("%s HTTP Response code: %d", TAG, httpResponseCode);
    const String payload = https.getString();

    // Free resources
    https.end();

    return std::make_optional(std::move(payload));
}
