// ======================================================
// Webserver program that gets temperature info
// from a Dallas DS18B20 and puts that in a linegraph
// on a webpage with a update every x seconds
// ======================================================

#include <Arduino.h>
#include <WiFi.h>

#include <ESPAsyncWebServer.h>

#include "wifisecrets.h"
#include "index.h"
#include "stubs.h"

// Setup a oneWire instance
// OneWire oneWire(ONE_WIRE_BUS);
Dummy_OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
// DallasTemperature sensors(&oneWire);
Dummy_DallasTemperature sensors(&oneWire);
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
  sensors.requestTemperatures();

  aRequest->send_P(200, "text/html", index_html_template, processor);
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
  Serial.print("processor aVar = ");
  Serial.println(aVar);

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
    Serial.println(pointsStr);
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
    Serial.print("The temperature is: ");
    Serial.print(static_cast<int>(sensors.getTempCByIndex(0)));
    Serial.println(" degrees C");
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
                                                                                          ->getPage()
********************/

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

// ===================================================
// Setup
// ===================================================
void setup()
{
  constexpr unsigned long SERIAL_MONITOR_SPEED{115200};
  Serial.begin(SERIAL_MONITOR_SPEED);

  pinMode(BUILDIN_LED_PIN, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(wifi::ssid);

  WiFi.begin(wifi::ssid, wifi::password);
  constexpr uint32_t WIFI_RECON_DELAY_MILLISEC{500};
  while (WL_CONNECTED != WiFi.status())
  {
    delay(WIFI_RECON_DELAY_MILLISEC);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  // Start up the library
  sensors.begin();
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
  if (newmil >= oldmil + UPDATE_INTERVAL_MILLISEC)
  {
    int tempC = sensors.getTempCByIndex(0U);

    Serial.print("The temperature in ticker is: ");
    Serial.print(tempC);
    Serial.println(" degrees C");

    digitalWrite(BUILDIN_LED_PIN, ledState);
    ledState = 1 - ledState;

    AddNewMeasurement(tempC);
    oldmil = newmil;
  }
}