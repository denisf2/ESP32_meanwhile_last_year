### What is this?
A pet project based on the ESP32-WROOM (with onboard WiFi). The ESP32 has a web server that displays weather information (mainly temperature) as a graph. There is no particular application.

### Why?
Just to try making an IoT device. A fusion of C++/Arduino framework, HTML, CSS, and JS under restricted conditions.

### How does it work?
On the first run, the ESP32 enables a WiFi access point and web server. You need to connect to its SSID: "esp32" (password: "esp32pass") and configure the settings to connect the device to your local WiFi network. To make the ESP32 work properly, you must add API keys for online data provider services on the corresponding web page. That's it! Now it shows weather information for your location based on your IP address or manually entered coordinates.

### What about the graph?
It displays the current temperature history for the last 3 days, along with the min/max temperatures for the same day last year (for 3 days before and after).

![screenshot](images/chart.png)
![screenshot](images/settings.png)
![screenshot](images/system.png)