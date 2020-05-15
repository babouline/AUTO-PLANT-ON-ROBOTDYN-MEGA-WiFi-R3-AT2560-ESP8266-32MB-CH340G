   #include <Wire.h>
   #include "DHT.h"
   
   #define I2CAddressESPWifi 8
   #define REDLED 25
   #define GREENLED 24
   #define FLOAT 26
   #define PUMP 27
   #define DHTPIN 23 
   #define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
   
int x;
int y;
int moistureLevel;
int moistureLevelTarget;
float u ;
int sens = 1; // reglage sensibilité
float v;
int sensorTemperature;
int sensorHumidity;
bool waterEmpty;
bool waterSwitchAuto;
bool waterSwitchEnabled;
int switcho;

void setup() {
  // put your setup code here, to run once:
Serial.begin (115200);
  dht.begin();
  pinMode(GREENLED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(FLOAT, INPUT); 
  waterEmpty = true;
    Wire.begin(I2CAddressESPWifi);
    Wire.onReceive(espWifiReceiveEvent);
    Wire.onRequest(espWifiRequestEvent);
  
}

void loop() 
{
  // put your main code here, to run repeatedly:
      delay(1000);
     readTemperatureHumidity();
     relevetemperature();
     readWaterLevel();
     updateLEDs(); 
     controlPump();
 }

 void relevetemperature()
 {
 x = analogRead(A0);
    Serial.print( x );
  
   if ( x < sens ){
     moistureLevel = 101;
     }
   if ( x >= sens ) {
    y = 1024 - x ;
     u = 1024 - sens;
     v = 100 / u;
       moistureLevel = y * v;
  
  }
  Serial.print("calcul ");//aide au debugage
  Serial.println( u );
  Serial.print("moisturelevel ");//aide au debugage
    Serial.println( moistureLevel );

 }
 
 void readTemperatureHumidity()   
   { 
    delay(1000);
    sensorHumidity = dht.readHumidity(); 
      Serial.println(sensorHumidity);//aide au debugage
      sensorTemperature = dht.readTemperature();
      Serial.println(sensorTemperature);//aide au debugage
    // if humidity read failed, don't change h value 
    if (isnan(sensorHumidity ) && isnan(sensorTemperature))
    {
      Serial.println("Failed to read from DHT sensor!");
        }
   }

void readWaterLevel()
{
  //  read water level sensor // j ai repris sa logique
  if ( moistureLevel < moistureLevelTarget ) {
    waterEmpty = true;
    Serial.println("recu beseoin d eau");
  } else {
    waterEmpty = false;
    Serial.println("recu j ai pas soif");
  }
}

void updateLEDs() {
  // Control green led
  if (waterSwitchAuto == 1)
  {
    digitalWrite(GREENLED, HIGH);
     Serial.println("switch auto"); // aide au debugage
    
  }
  else
  {
    digitalWrite(GREENLED, LOW);
     Serial.println("switch off"); // aide au debugage
  }
  // Control red led
  if (waterEmpty)
  {
    digitalWrite(REDLED, HIGH);
  }
  else
  {
    digitalWrite(REDLED, LOW);
  }  
}

void pompemanuel() {

  switcho = digitalRead(FLOAT);
  if ( (switcho) == LOW ) {
    waterSwitchEnabled == true;
    Serial.println("switchok");
  }
  if ( (switcho) == HIGH ) {
    waterSwitchEnabled == false;
     Serial.println("switchnok");
  }
  }

  void controlPump() {

    pompemanuel();
    // Control pump
  if (waterSwitchAuto && waterEmpty ) {
    digitalWrite(PUMP, HIGH);
    Serial.println("pompe on"); // aide au debugage
    delay(2000);
    digitalWrite(PUMP, LOW);
  }
   if ( waterEmpty && !waterSwitchAuto && ((switcho) == LOW) ){
     digitalWrite(PUMP, HIGH);
    Serial.println("pompe on"); // aide au debugage
  }
  else {
    digitalWrite(PUMP, LOW);
     Serial.println("pompe off"); // aide au debugage
  }
}
   
void espWifiReceiveEvent(int count)
    {
    Serial.println("Recu");
    while (Wire.available())
    {
      moistureLevelTarget = Wire.read();
      Serial.println(moistureLevelTarget);//aide au debugage
      waterSwitchAuto = Wire.read();
      Serial.println(waterSwitchAuto);//aide au debugage
      int c = Wire.read();
      Serial.println(c);
    }
    }
    void espWifiRequestEvent()
    {
         Wire.write(moistureLevel);
      Wire.write(sensorTemperature);
      Wire.write(sensorHumidity);
    }
    
