#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <PubSubClient.h>

/**
 *  Device Specific Config
 ***********************************************************/
#define TOKEN "ESP8266_TBUG_2"
IPAddress staticIP(192, 168, 8, 202); //ESP static ip
/***********************************************************/

#define SLEEPTIME 60*1e6  // DeepSleept for 60 seconds

// DHT
#define DHTPIN 2
#define DHTTYPE DHT11

// Wifi
#define WIFI_AP "Oaks-IOT"
#define WIFI_PASSWORD ""

// Use static IP to THingsboard server
char thingsboardServer[] = "192.168.8.101";

//Static network routing configuration
IPAddress gateway(192, 168, 8, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns1(192, 168, 8, 1);  // DNS1
const char* deviceName = TOKEN;

// The ESP8266 RTC memory is arranged into blocks of 4 bytes. The access methods read and write 4 bytes at a time,
// so the RTC data structure should be padded to a 4-byte multiple.
struct {
  uint32_t crc32;   // 4 bytes
  uint8_t channel;  // 1 byte,   5 in total
  uint8_t bssid[6]; // 6 bytes, 11 in total
  uint8_t padding;  // 1 byte,  12 in total
} rtcData;

WiFiClient wifiClient;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

boolean getTemperatureAndHumidityData(char* attributes)
{

  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  Serial.print("Raw H: ");
  Serial.println(h);
  Serial.print("Raw T: ");
  Serial.println(t);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  String temperature = String(t);
  String humidity = String(h);


  // Just debug messages
  Serial.print( "Constructing temperature and humidity payload : [" );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"humidity\":"; payload += humidity;
  payload += "}";

  // Send payload

  payload.toCharArray( attributes, 100 );

  return true;
}

uint32_t calculateCRC32( const uint8_t *data, size_t length ) {
  uint32_t crc = 0xffffffff;
  while( length-- ) {
    uint8_t c = *data++;
    for( uint32_t i = 0x80; i > 0; i >>= 1 ) {
      bool bit = crc & 0x80000000;
      if( c & i ) {
        bit = !bit;
      }

      crc <<= 1;
      if( bit ) {
        crc ^= 0x04c11db7;
      }
    }
  }

  return crc;
}

int InitWiFi()
{

  // Try to read WiFi settings from RTC memory
  bool rtcValid = false;
  if( ESP.rtcUserMemoryRead( 0, (uint32_t*)&rtcData, sizeof( rtcData ) ) ) {
    // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
    uint32_t crc = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    if( crc == rtcData.crc32 ) {
      rtcValid = true;
    }
  }

  Serial.println("Connecting to AP via static IP ...");

  // For disconnect to clear old Wifi settings stored in Flash. Only do this once.
  // WiFi.disconnect();

  // turn radio ON
  WiFi.forceSleepWake();
  delay( 1 );

  // reduce wear and don't persist connection info to flash memory
  WiFi.persistent(false);
  // don't re-connect on restart
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  // limit to Wifi Station mode (connect to wifi router only) 
  WiFi.mode(WIFI_STA);
  // set hostname for easy identification 
  WiFi.hostname(deviceName);

  // attempt to connect to WiFi network
  while (!WiFi.config(staticIP, gateway, subnet, dns1)) {
    Serial.println("Unable to config for Static IP!  Keep looping...");
    delay(500);
  }

  if( rtcValid ) {
    // The RTC data was good, make a quick connection
    WiFi.begin( WIFI_AP, WIFI_PASSWORD, rtcData.channel, rtcData.bssid, true );
  }
  else {
    // The RTC data was not valid, so make a regular connection
    WiFi.begin( WIFI_AP, WIFI_PASSWORD );
  }


  int retries = 0;
  status = WiFi.status();
  while (status != WL_CONNECTED) {
    retries++;
    Serial.print(".");

    if( retries == 100 ) {
      // Quick connect is not working, reset WiFi and try regular connection
      WiFi.disconnect();
      delay( 10 );
      WiFi.forceSleepBegin();
      delay( 10 );
      WiFi.forceSleepWake();
      delay( 10 );
      WiFi.begin( WIFI_AP, WIFI_PASSWORD );
    }
    if( retries == 600 ) {
      // Giving up after 30 seconds and going back to sleep
      WiFi.disconnect( true );
      delay( 1 );
      WiFi.mode( WIFI_OFF );
      ESP.deepSleep( SLEEPTIME, WAKE_RF_DISABLED );
      return status; // Not expecting this to be called, the previous call will never return.
    }

    delay(50);
    status = WiFi.status();
  }

  if (status == WL_CONNECTED){
    Serial.print("Connected to AP. IP: ");
    Serial.print(WiFi.localIP());
    Serial.println("  [DONE]");

    // Write current connection info back to RTC
    rtcData.channel = WiFi.channel();
    memcpy( rtcData.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
    rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    ESP.rtcUserMemoryWrite( 0, (uint32_t*)&rtcData, sizeof( rtcData ) );

  } else {
    Serial.println("Wifi init failed!");
  }

  return status;
}


void reconnect() {
  int count = 0;
  // Loop until we're reconnected
  while (!client.connected() && count++ <= 2) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      Serial.println("Wifi not connected. Re-init Wifi.");
      status = InitWiFi();
    }

    if (status == WL_CONNECTED) {
      Serial.print("Connecting to Thingsboard node at: ");
      Serial.print(thingsboardServer);
      Serial.print(" ....");
      // Attempt to connect (clientId, username, password)
      if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
        Serial.println( "[DONE]" );
      } else {
        Serial.print( "[FAILED] [ rc = " );
        Serial.print( client.state() );
        Serial.println( " : retrying in 1 seconds]" );
        // Wait 1 seconds before retrying
        delay( 1000 );
      }
    } else {
      Serial.print("Not connected to Wifi, cannot connect to Thingsboard. Count=");
      Serial.println(count);
      // Wait 1 seconds before retrying
      delay( 1000 );
    }

  }
}

void setup()
{

  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay( 1 );

  dht.begin();

  Serial.begin(115200);
  // wait for Serial to be up
  while(!Serial) {}

  char attributes[100];

  delay(2000);

  // first read sensor data
  if (getTemperatureAndHumidityData(attributes)) {

    Serial.print("Got payload: ");
    Serial.println( attributes );

    client.setServer( thingsboardServer, 1883 );
    if ( !client.connected() ) {
      reconnect();
    }

    client.publish( "v1/devices/me/telemetry", attributes );

    delay(100);
  }

  Serial.println("Going to DeepSleep...");
  WiFi.disconnect( true );
  delay( 1 );
  // sleep for 60 seconds
  ESP.deepSleep(SLEEPTIME, WAKE_RF_DISABLED);

}

void loop()
{

}