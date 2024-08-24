/* VEML7700 Auto Lux Example
 *
 * This example sketch demonstrates reading lux using the automatic
 * method which adjusts gain and integration time as needed to obtain
 * a good reading. A non-linear correction is also applied if needed.
 *
 * See Vishy App Note "Designing the VEML7700 Into an Application"
 * Vishay Document Number: 84323, Fig. 24 Flow Chart
 */

#include "Adafruit_VEML7700.h"

#define RXD1 16
#define TXD1 17

Adafruit_VEML7700 veml = Adafruit_VEML7700();

void setup() {
  Serial.begin(115200);
  // Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);
  while (!Serial) { delay(10); }
  Serial.println("Adafruit VEML7700 Auto Lux Test");

  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");

  pinMode(4, OUTPUT);    // sets the digital pin 4 as output to control emission LED 
  digitalWrite(4, HIGH);  // sets the digital pin 4 off to turn the emission LED off

}

void loop() {
  // to read lux using automatic method, specify VEML_LUX_AUTO
  float lux = veml.readLux(VEML_LUX_AUTO);
  Serial.println(lux);
  //digitalWrite(4, !digitalRead(4));  // blink the emission LED 
  //delay(lux/100);           
  /*Serial1.println(lux);
  if (Serial1.available()) {
    // int inByte = Serial1.parseFloat();//Serial1.read();
    //Serial.write("   got something");
    Serial.print(Serial1.parseFloat());//(inByte);
    Serial.println();
  }*/
  //Serial1.readBytes();
  /*Serial.println("------------------------------------");
  Serial.print("Lux = "); Serial.println(lux);
  Serial.println("Settings used for reading:");
  Serial.print(F("Gain: "));
  switch (veml.getGain()) {
    case VEML7700_GAIN_1: Serial.println("1"); break;
    case VEML7700_GAIN_2: Serial.println("2"); break;
    case VEML7700_GAIN_1_4: Serial.println("1/4"); break;
    case VEML7700_GAIN_1_8: Serial.println("1/8"); break;
  }
  Serial.print(F("Integration Time (ms): "));
  switch (veml.getIntegrationTime()) {
    case VEML7700_IT_25MS: Serial.println("25"); break;
    case VEML7700_IT_50MS: Serial.println("50"); break;
    case VEML7700_IT_100MS: Serial.println("100"); break;
    case VEML7700_IT_200MS: Serial.println("200"); break;
    case VEML7700_IT_400MS: Serial.println("400"); break;
    case VEML7700_IT_800MS: Serial.println("800"); break;
  }*/


  delay(1000);
}