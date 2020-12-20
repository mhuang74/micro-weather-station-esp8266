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

## Notes
* Rust
    * Got as far as Blink, but unable to successfully send logs over Serial
    * Abandoned after firmware seems corrupted after espflash; bootloader reports exception over Serial
    * Overall, toolchain seems not mature enough yet to use in project...
* Arduino
    * Arduino libraries and example code makes it much easier to work in Arduino land. But instead of using Arduino IDE, Platformio + VSCode takes the whole dev experience to the next level!



