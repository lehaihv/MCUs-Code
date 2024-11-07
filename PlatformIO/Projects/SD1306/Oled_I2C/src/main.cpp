#include <Arduino.h>
#include "Adafruit_VEML7700.h"

Adafruit_VEML7700 veml = Adafruit_VEML7700();

float lux = 0;

// Initialize VEML7700 sensor
void Init_VEML7700_Sensor() {
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
   // Check VEML7700 Sensor is ready 
  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");
  // Init VEML7700 Sensor
  Init_VEML7700_Sensor(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  lux = veml.readLux();  // Measure the fluorescence intensity //VEML_LUX_AUTO
  Serial.println(lux);
  delay(1000);
}


