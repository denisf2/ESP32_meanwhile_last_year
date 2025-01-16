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

constexpr int listenPort{80};
AsyncWebServer server(listenPort);

// #include <OneWire.h>
// #include <DallasTemperature.h>

// ---------------------------------------------------
// dummy stub
// ---------------------------------------------------
#define ONE_WIRE_BUS 23

class Dummy_OneWire
{
public:
  Dummy_OneWire(int _tmp) {}
};

class Dummy_DallasTemperature
{
private:
  mutable unsigned int m_value{0};

public:
  Dummy_DallasTemperature(const Dummy_OneWire *const _tmp) {}
  void requestTemperatures() const {}
  int getTempCByIndex(uint _aIndex) const
  {
    m_value = std::min(m_value + 5, 100U);
    return m_value;
  }
  void begin() {}
};

// Setup a oneWire instance
// OneWire oneWire(ONE_WIRE_BUS);
Dummy_OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
// DallasTemperature sensors(&oneWire);
Dummy_DallasTemperature sensors(&oneWire);
// ---------------------------------------------------

constexpr size_t arraySize{10};
int tempArray[arraySize] = {};
String timeArray[arraySize] = {};
unsigned long oldmil = 0UL;
const unsigned long UPDATE_INTERVAL = 10000UL;

constexpr uint8_t BUILDIN_LED_PIN{2};
uint8_t ledState{static_cast<uint8_t>(LOW)};

String processor(const String &aVar);

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
  Serial.println(aVar);

  if (aVar.equals("ARRAYPLACEHOLDER"))
  {
    String buttons;
    for (size_t i = 0; i < arraySize; ++i)
    {
      buttons += ",[";
      buttons += String(timeArray[i]);
      buttons += ",";
      buttons += String(tempArray[i]);
      buttons += "]";
    }
    return buttons;
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

// handleRoot->has arguments->handleSubmit->has "button1"->update sensor data
//                          |--------------|--------------|-----------------|->getPage()
//  onNotFound->handleNotFound
//                           |->getPage()

// ===================================================
// Update values history
// ===================================================
auto UpdateFIFO(int aNewVAlue) -> void
{
  constexpr size_t size{arraySize - 1};
  for (size_t i = 0; i < size; i++)
  {
    tempArray[i] = tempArray[i + 1];
    timeArray[i] = timeArray[i + 1];
  }

  tempArray[size] = aNewVAlue;
  timeArray[size] = String(static_cast<int>(millis() / 1000UL));
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
  constexpr uint32_t WIFI_RECON_DELAY_MSEC{500};
  while (WL_CONNECTED != WiFi.status())
  {
    delay(WIFI_RECON_DELAY_MSEC);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // server.begin();
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

  const auto newmil = millis();
  if (newmil >= oldmil + UPDATE_INTERVAL)
  {
    int temptemp = static_cast<int>(sensors.getTempCByIndex(0U));

    Serial.print("The temperature in ticker is: ");
    Serial.print(temptemp);
    Serial.println(" degrees C");

    digitalWrite(BUILDIN_LED_PIN, ledState);
    ledState = 1 - ledState;

    UpdateFIFO(temptemp);
    oldmil = newmil;
  }
}