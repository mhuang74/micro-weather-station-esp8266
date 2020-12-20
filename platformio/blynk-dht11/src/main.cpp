/* Comment this out to disable prints and save space */
// #define BLYNK_PRINT Serial

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>

//// Use Blynk Static Provisioning since only expect 4 devices for my home micro weather station
// Auth Token for device 'Sta-1' : e5rGLloeOHCGjNomaR0KNbfHXSY1fVap
// Auth Token for device 'Sta-2' : a34l0ByU7ZeeQOrFla5lfbGqewr6mZI_
// Auth Token for device 'Sta-3' : CMg96HTDs7uTqv3LD6lmTSndMsIDCWgn
// Auth Token for device 'Sta-4' : L155ijUo03vRHYH1oasS3i-LYe0hCmcP

#define AUTH_STA1 "e5rGLloeOHCGjNomaR0KNbfHXSY1fVap"
#define AUTH_STA2 "a34l0ByU7ZeeQOrFla5lfbGqewr6mZI_"
#define AUTH_STA3 "CMg96HTDs7uTqv3LD6lmTSndMsIDCWgn"
#define AUTH_STA4 "L155ijUo03vRHYH1oasS3i-LYe0hCmcP"

char auth[] = AUTH_STA1;

// Use IOT Wifi Guest SSID on Asus

char ssid[] = "Oaks-IOT";
char pass[] = "";

#define DHTPIN 2          // ESP01S-DHT11 shield uses GPIO2 for sensor data
#define ONBOARD_LED  3

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

// Initialize DHT sensor 
DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;

int rowIndex = 0;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Temperature (C): ");
  Serial.println(t);

  // Humidity on Virutal Pin 5
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V5, "add", rowIndex, "Humidity (S2)", h);
  //highlighting latest added row in table
  Blynk.virtualWrite(V5, "pick", rowIndex);
  rowIndex++;

  // Temperature (C) on Virtual Pin 6
  Blynk.virtualWrite(V6, t);
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  pinMode(ONBOARD_LED,OUTPUT);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  //clean Humdity table at start
  Blynk.virtualWrite(V5, "clr");

  dht.begin();

  // Setup a function to be called every 10 seconds
  timer.setInterval(10000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();

  digitalWrite(ONBOARD_LED,HIGH);
  delay(200);
  digitalWrite(ONBOARD_LED,LOW);
}
