# micro-weather-station-esp8266
Simple esp8266 Microcontroller Project for monitoring Humidity and Temperature

## Main Components

* Sensors
    * DHT11 ESP01 shield
    * BME280 (more accurate), which can be connected directly via I2C protocol
* Hardware: 
    * ESP8266 via ESP-01S board
* Telemetrics: 
    * mqtt
    * Thingsboard Dashboards
* Language: 
    * C with Arduino libraries
* Dev Environment
    * Platformio with VSCode

## Pivots
* Rust -> C
    * Got as far as Blink using espflash docker image, but unable to successfully send logs over Serial
    * Abandoned after firmware seems corrupted after espflash; bootloader reports exception over Serial
    * Overall, toolchain seems not mature enough yet to use in project...
* Arduino IDE -> Platformio + VSCode
    * Arduino libraries and example code makes it much easier to work in Arduino land. But instead of using Arduino IDE, Platformio + VSCode takes the whole dev experience to the next level!
* Blynk -> Thingsboard
    * Blynk has a great App to look at collected data, but Thingsboard provides a very flexible web-based Dashboard, as well as ability to define Rules and Actions (eg. send email alert if humidify > 90%).
    * Thingsboard is industrial strength, has a free Community Edition. Looking forward to do real projects with it.



