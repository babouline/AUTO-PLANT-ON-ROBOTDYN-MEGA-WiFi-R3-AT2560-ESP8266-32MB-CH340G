#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WIFI SSID & PASS *********************************/
#define I2CAddressESPWifi 8
#define WLAN_SSID       ".." // adresse
#define WLAN_PASS       ".." //  mot de passe

/************************* ADAFRUIT AUTHORISATION *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883 // use 8883 for SSL
#define AIO_USERNAME    ".." // user adafruit
#define AIO_KEY         ".."  // key adafruit

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** ADAFRUIT FEEDS ***************************************/

Adafruit_MQTT_Publish moisture = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisture");
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature"); 
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish messages = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/messages");

Adafruit_MQTT_Subscribe waterswitch = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/waterswitch");
Adafruit_MQTT_Subscribe niveau = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/niveau");
void MQTT_connect();

/*************************** SETUP ************************************/

Adafruit_MQTT_Subscribe *subscription;
bool waterEmpty;
bool waterSwitchAuto;
int moistureLevel;
int moistureLevelTarget;
int sensorTemperature;
int sensorHumidity;

void setup() {
  //  Define inputs & outputs
  Serial.begin(115200);
  Serial.print("Executing code...");


  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  mqtt.subscribe(&waterswitch);
  mqtt.subscribe(&niveau);

  waterEmpty = true;
  waterSwitchAuto = false;
  moistureLevelTarget = 0;
  
  Wire.begin(0,2);//Change to Wire.begin() for non ESP.
}

void loop() {
  // connect
  MQTT_connect();
  
  // read
  readarduino();
  readAdafruitSubscriptions();
  readWaterLevel();
   publishStatusAdafruit();
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

void readarduino()
{ 
 
Wire.requestFrom(I2CAddressESPWifi,3);
      while (Wire.available())
    {
    delay(1);
      moistureLevel = Wire.read();
      Serial.println(moistureLevel);//aide au debugage
      sensorTemperature = Wire.read();
      Serial.println(sensorTemperature);//aide au debugage
      sensorHumidity = Wire.read();
      Serial.println(sensorHumidity);
      
    }
   Wire.beginTransmission(I2CAddressESPWifi);
    Wire.write(moistureLevelTarget);
    Wire.write(waterSwitchAuto);
    Wire.write(sensorHumidity);
    Wire.endTransmission();
        
    delay(5000);
   }


void readWaterLevel()
{
  //  read water level sensor // j ai repris sa logique
  if ( moistureLevel < moistureLevelTarget ) {
    waterEmpty = true;
  } else {
    waterEmpty = false;
  }
}

void readAdafruitSubscriptions() 
{
    // Read Adafruit subscriptions and update internal variables
  subscription = mqtt.readSubscription(5000);
  if (subscription == &waterswitch) {
    if (strcmp((char *)waterswitch.lastread, "Auto") == 0) {
      waterSwitchAuto = true;
    } else {
      waterSwitchAuto = false;
    }
  }
  else if (subscription == &niveau) {
    uint16_t levelval = atoi((char *)niveau.lastread);  // convert to a number
    moistureLevelTarget = levelval;
    //Serial.print("target");//aide au debugage
    //Serial.println( levelval );//aide au debugage
  }
}

void publishStatusAdafruit() {
  // Publish sensor values to Adafruit
  if (!waterEmpty && waterSwitchAuto)
  {
    messages.publish("niveau d eau ok");
  }
  else if (waterEmpty && waterSwitchAuto )
  {
    messages.publish("Remplissage en cours");
      }
        else if (waterEmpty && !waterSwitchAuto)
  {
    messages.publish("activer le remplissage");
  }
  else
  {
    messages.publish("mode manuel");
  }
  moisture.publish(moistureLevel);
  temperature.publish(sensorTemperature);
  humidity.publish(sensorHumidity);
}
