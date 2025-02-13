#include "WebServerHandlers.h"

#include "resources.h"

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
