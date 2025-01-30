// ======================================================
// Webserver program that gets temperature info
// from a Dallas DS18B20 and puts that in a linegraph
// on a webpage with a update every x seconds
// ======================================================

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "NvsPreferences.h"
#include "wifisecrets.h"
#include "stubs.h"
#include "resources.h"
#include "TLog.h"

// [ ]TODO: remove dummy stubs
// Setup a oneWire instance
// OneWire oneWire(ONE_WIRE_BUS);
Dummy_OneWire __oneWire(ONE_WIRE_BUS);// [ ]TODO: remove

// Pass our oneWire reference to Dallas Temperature.
// DallasTemperature sensors(&oneWire);
Dummy_DallasTemperature __sensors(&__oneWire);// [ ]TODO: remove
// ---------------------------------------------------

constexpr int listenPort{80};
AsyncWebServer server(listenPort);

constexpr size_t arraySize{10};
int tempArray[arraySize] = {};
unsigned long timeArray[arraySize] = {};
unsigned long oldmil = 0UL;
const unsigned long UPDATE_INTERVAL_MILLISEC = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};
// ---------------------------------------------------

String processor(const String &aVar);
// ---------------------------------------------------

void GetPage(AsyncWebServerRequest *aRequest)
{
  __sensors.requestTemperatures();

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
void handleNotFound(AsyncWebServerRequest *aRequest)
{
  GetPage(aRequest);
}

// функция формирования содержимого WEB страницы
String processor(const String &aVar)
{
  TLog::print("processor aVar = ");
  TLog::println(aVar);

  if (aVar.equals("ARRAYPLACEHOLDER"))
  {
    String pointsStr;
    for (size_t i = 0; i < arraySize; ++i)
    {
      pointsStr += ",[";
      pointsStr += String(timeArray[i]);
      pointsStr += ",";
      pointsStr += String(tempArray[i]);
      pointsStr += "]";
    }
    TLog::println(pointsStr);
    return pointsStr;
  }

  return String();
}

// ==================================================
// Handle submit form
// ==================================================
void handleSubmit(AsyncWebServerRequest *aRequest)
{
  if (aRequest->hasArg("button1"))
  {
    TLog::print("The temperature is: ");
    TLog::print(static_cast<int>(__sensors.getTempCByIndex(0)));
    TLog::println(" degrees C");
  }

  GetPage(aRequest); // Response to the HTTP request
}

// ===================================================
// Handle root
// ===================================================
void handleRoot(AsyncWebServerRequest *aRequest)
{
  if (aRequest->args())
  {
    handleSubmit(aRequest);
  }
  else
  {
    GetPage(aRequest);
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

auto HandleFavIcon(AsyncWebServerRequest *aRequest) -> void
{
  TLog::println("Requested favicon.ico");

  auto response = aRequest->beginResponse_P(200
                                          , "image/x-icon"
                                          , favicon_ico_gz_start
                                          , favicon_ico_gz_size
                                          // , processor
                                                );
  response->addHeader("Content-Encoding", "gzip");
  aRequest->send(response);
}

auto HandleUpdateParams(AsyncWebServerRequest *aRequest) -> void
{
  TLog::println("HandleUpdateParams. Params total: ");
  const size_t paramsNr = aRequest->params();
  TLog::print(paramsNr);

  for (size_t i = 0; i < paramsNr; i++)
  {
    const AsyncWebParameter * const p = aRequest->getParam(i);
    TLog::println("Param [name, value] : [");
    TLog::print(p->name());
    TLog::print(", ");
    TLog::print(p->value());
    TLog::print("]");
  }

  GetPage(aRequest);
}

// ===================================================
// Update values history
// ===================================================
auto AddNewMeasurement(int aNewVAlue) -> void
{
  constexpr size_t size{arraySize - 1};
  timeArray[size] = millis() / 1000UL;

  for (size_t i = 0; i < size; i++)
  {
    tempArray[i] = tempArray[i + 1];
    timeArray[i] = timeArray[i + 1];
  }

  tempArray[size] = aNewVAlue;
}

auto LockingWiFiConnection() -> void
{
  // Connect to Wi-Fi network with SSID and password
  TLog::println("Connecting to ");
  TLog::print(wifi::ssid);
  TLog::print(" ");

  WiFi.begin(wifi::ssid, wifi::password);
  constexpr uint32_t WIFI_RECON_DELAY_MILLISEC{500};
  while (WL_CONNECTED != WiFi.status())
  {
    delay(WIFI_RECON_DELAY_MILLISEC);
    TLog::print(".");
  }

  // Print local IP address and start web server
  TLog::println("WiFi connected.");
  TLog::println("IP address: ");
  TLog::print(WiFi.localIP());
  TLog::println();
}



  {
  }
}

// ===================================================
// Setup
// ===================================================
void setup()
{
  constexpr unsigned long SERIAL_MONITOR_SPEED{115200};
  Serial.begin(SERIAL_MONITOR_SPEED);

  TLog::println("=================================");

  pinMode(BUILDIN_LED_PIN, OUTPUT);

  // [ ]TODO: setup and run default wifi access point on cold start
  RestoreStoredData();
  LockingWiFiConnection();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_GET, HandleUpdateParams);
  server.on("/favicon.ico", HTTP_GET, HandleFavIcon);
  server.onNotFound(handleNotFound);
  server.begin();

  // Start up the library
  __sensors.begin();
}

// ===================================================
// Loop
// ===================================================
void loop()
{
  // server.handleClient();
  // server.send(200, "text/html", getPage());
  // * must send data by websocket

  const auto newmil = millis();
  if (newmil - oldmil >= UPDATE_INTERVAL_MILLISEC)
  {
    int tempC = __sensors.getTempCByIndex(0U);

    TLog::println("The temperature in ticker is: ");
    TLog::print(tempC);
    TLog::print(" degrees C");

    digitalWrite(BUILDIN_LED_PIN, ledState);
    ledState = 1 - ledState;

    AddNewMeasurement(tempC);
    oldmil = newmil;
  }
}

/* Roadmap:

[ ]TODO: get gps coordinates by ip
[ ]TODO: get forecast by gps coordinates
[ ]TODO: update wifi params from webpage
[ ]TODO: update api keys params from webpage
[ ]TODO: visualizate forecast using google charts
[ ]TODO: export to a new module RestoreStoredData

*/
