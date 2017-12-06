
#define CAYENNE_DEBUG         // Uncomment to show debug messages
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#define MILLIS_INTERVAL 500

#include <CayenneMQTTMKR1000.h>
#include "settings.h"
#include <SparkFun_Si7021_Breakout_Library.h>
#include <Wire.h>
#include <MS5611.h>
// Cayenne authentication token. This should be obtained from the Cayenne Dashboard.

// Your network name and password.

//SI7021 si7021;
int occupancy_step = 0;
bool persistence[10] = {false, false, false, false, false, false, false, false, false, false};
bool console_connected = false;
unsigned long lastMillis;
int iSeeYou = 0;
int pirStatus = 0;
int lastPirStatus = 0;
float humidity = 0;
float temp = 0;
float pressure = 0;
Weather sensor;
MS5611 Psensor(&Wire);
int oc = 0;
int led = 9;

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIR_DOUT, INPUT);
  Serial.begin(9600);
  sensor.begin();
  Psensor.connect();
  lastMillis = millis();
  Cayenne.begin(mqttUser, mqttPass, mqttID, ssid, pass);
  Cayenne.virtualWrite(4, 0, 0, "null");
  if(Serial)
  {
    console_connected = true;
  }
}

void loop()
{
  if(Serial && console_connected == false)
  {
    console_connected = true;
    Serial.println("Serial terminal connected");
  }
  getWeather();
  oc = digitalRead(PIR_DOUT);
  if(oc)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);
  pirStatus = persistent_occupancy(oc);
  if(pirStatus == 0 && iSeeYou == 1)
  {
      iSeeYou = 0;
      digitalWrite(led, iSeeYou);
  }
   if(millis() - lastMillis > 500)
  {
    Cayenne.celsiusWrite(1, temp);
    Cayenne.virtualWrite(2, humidity, "rel_hum","p");
    Cayenne.virtualWrite(3, pressure, "bp", "pa");
    Cayenne.virtualWrite(5, pirStatus, "prox", "bool");
    Cayenne.virtualWrite(4, iSeeYou, 0 , "null");
    lastMillis = millis();
    lastMillis = millis();
  }
   Cayenne.loop();
}


void getWeather()
{
  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  temp = sensor.getTemp();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()

  Psensor.ReadProm();
  Psensor.Readout();
  pressure = Psensor.GetPres()/100.0;

}

CAYENNE_IN(4)
{
  iSeeYou = getValue.asInt();
  if(pirStatus != 1){
    iSeeYou = 0;
  }
  digitalWrite(led, iSeeYou);
}

int persistent_occupancy(int occupancy)
{
  int sum = 0;
  Serial.print("Occupancy: "); Serial.println(occupancy);
  persistence[occupancy_step] = occupancy;
  for(int i=0; i<10; i++){
    if(persistence[i] == true)
      sum += 1;
  }
  Serial.print("Sum:"); Serial.println(sum);
  occupancy_step = (occupancy_step + 1)%10;
  return (sum > 2);    
}

