#include <WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

const char* ssid = "EmilyandPony";   // Write your SSID
const char* password = "ngocdiep2015";   // Write your WIFI password

WiFiClient  client;

unsigned long Channel_ID = 2309847;  //replace with your Channel ID
const char * API_Key = "HNGLB2O3I6IXBHRI";


unsigned long last_time = 0;
unsigned long Delay = 5000;//30000;

// Variables to store sensor readings
float temperature;
float humidity;
float pressure;
float i = 0;


// Create a sensor object
Adafruit_BME280 bme; 

void initBME(){
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not properly connected!");
    while (1);
  }
}

void setup() {
  Serial.begin(115200);  
  //initBME();
  
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client); 
}

void loop() {
  if ((millis() - last_time) > Delay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Connecting...");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    // Obtaining a new sensor reading for all fields
    temperature = 25.5 + i;//bme.readTemperature();
    Serial.print("Temperature (ºC): ");
    Serial.println(temperature);
    humidity = 75.6 + i;//bme.readHumidity();
    Serial.print("Humidity (%): ");
    Serial.println(humidity);
    //pressure = bme.readPressure() / 100.0F;
    //Serial.print("Pressure (hPa): ");
    //Serial.println(pressure);
    
    ThingSpeak.setField(1, temperature);
    ThingSpeak.setField(2, humidity);
    //ThingSpeak.setField(3, humidity);
    i += 0.5;
    int Data = ThingSpeak.writeFields(Channel_ID, API_Key);

    if(Data == 200){
      Serial.println("Channel updated successfully!");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(Data));
    }
    last_time = millis();
  }
}