#include "WebServerHandlers.h"

#include "resources.h"
#include "NvsPreferences.h"

const char TAG[] = "[WebSrv]";

constexpr int listenPort{80};
AsyncWebServer server(listenPort);

auto HandleFavIcon(AsyncWebServerRequest *aRequest) -> void
{
    log_d("%s Requested favicon.ico", TAG);

    auto response = aRequest->beginResponse_P(200
                                            , "image/x-icon"
                                            , favicon_ico_gz_start
                                            , favicon_ico_gz_size
                                              // , processor
    );
    response->addHeader("Content-Encoding", "gzip");
    aRequest->send(response);
}

auto HandleDeviceSoftRestart(AsyncWebServerRequest *aRequest) -> void
{
    log_d("%s Handle Soft Restart", TAG);

    const char webpage[] PROGMEM = R"rawhtml(
        <!DOCTYPE HTML>
        <html>
        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0 maximum-scale=2.5, user-scalable=1">
            <title>Temperature Forecast Linechart demo</title>
        </head>
        <body>
        <h2>Restarting...</h2>
        </body>
        </html>
    )rawhtml";

    aRequest->send(200, String("text/html"), webpage);

    ESP.restart();
}

auto SendWebPageResponse(AsyncWebServerRequest *aRequest) -> void
{
    auto response = aRequest->beginResponse_P(200
                                            , "text/html"
                                            , index_html_gz_start
                                            , index_html_gz_size
                                            // , processor
                                            );
    response->addHeader("Content-Encoding", "gzip");
    aRequest->send(response);

    // aRequest->send_P(200, "text/html", index_html_template, processor);
}

// ==================================================
// Handle for page not found
// ==================================================
auto handleNotFound(AsyncWebServerRequest *aRequest) -> void
{
    SendWebPageResponse(aRequest);
}

auto HandleUpdateParams(AsyncWebServerRequest *aRequest) -> void
{
    const size_t paramsNr = aRequest->params();

    log_i("%s Params total: %d", TAG, paramsNr);

    for (size_t i = 0; i < paramsNr; i++)
    {
        const AsyncWebParameter *const p = aRequest->getParam(i);
        const auto name = p->name();
        const auto value = p->value();

        log_i("%s Param [name, value] : [%s, %s]", TAG, name.c_str(), value.c_str());

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

        if (name.equals("autolocation"))
            SaveAutoLocation(value.equals("true") ? true : false);
    }

    // [ ]TODO: why do we need send in response whole page?
    SendWebPageResponse(aRequest);
}

// ==================================================
// Handle submit form
// ==================================================
auto handleSubmit(AsyncWebServerRequest *aRequest) -> void
{
    // [ ]TODO: do something with updateChartButton
    if (aRequest->hasArg("updateChartButton"))
    {
        log_i("%s updateChartButton was pressed", TAG);
    }

    SendWebPageResponse(aRequest); // Response to the HTTP request
}

// ===================================================
// Handle root
// ===================================================
auto handleRoot(AsyncWebServerRequest *aRequest) -> void
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
handleRoot->has arguments->handleSubmit->has "updateChartButton"->print to serial sensor data
                         |              |              |                          |------>|
                         |              |              |--------------------------------->|
                         |              |------------------------------------------------>|
                         |--------------------------------------------------------------->|
                                                                                          |
onNotFound->handleNotFound                                                                |
                         |--------------------------------------------------------------->|
                                                                                          |->getPage()
********************/
