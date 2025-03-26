### What is this?
Pet project based on esp32-wroom (WiFi onboard). Esp32 has web server onboard which shows weather info (mainly temperature) as graph.

### Why?
Just to try make IoT. Fusion c++/arduino framework/html/css/js under restricted conditions.

### How does it work?
At the first run esp32 enables WiFi AP and web server. You have to connect to it SSID: "esp32" and password: "esp32pass" make some settings to connect device to your local WiFi network. To make esp32 work properly you have to add online data provider services api keys on corresponding web page. That is it! Now it shows info about weather at your location based on IP address or manualy added coordinates.

### What about is the graph?
It demonstrates current temperature back log for 3 days over min/max temperature on the same day last year for 3 days past and 3 days before.
### Pseudo graph
```
current     T     T     T     T
last year   Tmax  Tmax  Tmax  Tmax  Tmax  Tmax  Tmax
last year   Tmin  Tmin  Tmin  Tmin  Tmin  Tmin  Tmin
            day-3 day-2 day-1 today day+1 day+2 day+3
```