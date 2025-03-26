#include "WebServerHandlers.h"

#include "resources.h"
#include "NvsPreferences.h"

constexpr int listenPort{80};
AsyncWebServer server(listenPort);

auto HandleFavIcon(AsyncWebServerRequest *aRequest) -> void
{
    log_d("Requested favicon.ico");

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
    log_d("Handle Soft Restart");

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

    log_i("Params total: %d", paramsNr);

    for (size_t i = 0; i < paramsNr; i++)
    {
        const AsyncWebParameter *const p = aRequest->getParam(i);
        const auto name = p->name();
        const auto value = p->value();

        log_i("Param [name, value] : [%s, %s]", name.c_str(), value.c_str());

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
    }

    // [ ]TODO: why do we need send in response whole page?
    SendWebPageResponse(aRequest);
}

// ==================================================
// Handle submit form
// ==================================================
auto handleSubmit(AsyncWebServerRequest *aRequest) -> void
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
