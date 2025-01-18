/*

*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

/* #define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10 */

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 BME280; // I2C
// Adafruit_BME280 bme(BME_CS); // hardware SPI
// Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// define pin for I2C module 1 ---> BME280
#define I2C_0_SDA 8
#define I2C_0_SCL 9

// define pin for I2C module 2 ---> OLED display
#define I2C_1_SDA 7
#define I2C_1_SCL 15

TwoWire I2C_BME280 = TwoWire(0); // "0", "1" instance of I2C module bus
TwoWire I2C_OLED = TwoWire(1);

// Declaration for an SSD1306 display connected to I2C1 (SDA1, SCL1 pins)
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, 5); //, -1 if use RTS of ESP32-S3);

// Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

/**************************************************************************/
/*
    Program entry point for the Arduino sketch
*/
/**************************************************************************/
void setup(void)
{
  // disableBuiltinLED();

  Serial.begin(115200);
  // pinMode(4, OUTPUT);     // sets the digital pin 4 as output to control emission LED
  // digitalWrite(4, LOW);
  I2C_BME280.begin(I2C_0_SDA, I2C_0_SCL);
  I2C_OLED.begin(I2C_1_SDA, I2C_1_SCL); // (I2C_1_SDA, I2C_1_SCL, 100000);
  // Oled initial
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64 0x3C (0x78/0x79 for SSD1309 Oled)
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, world!");
  display.display();

  // BME280 Init
  Serial.println(F("BME280 test"));
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!BME280.begin(0x76, &I2C_BME280))  // 0x77
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(BME280.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1)
      delay(10);
  }
  Serial.println("-- Default Test --");
  Serial.println();
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/

void loop(void)
{
  display.clearDisplay();
  display.setCursor(10, 10); // (X,Y)
  display.println("Inside condition");
  display.println();
  display.print("Temperature: "); display.print(BME280.readTemperature()); display.println(" °C"); 
  display.print("TPressure: "); display.print(BME280.readPressure() / 100.0F); display.println(" hPa"); 
  display.print("Approx. Altitude: "); display.print(BME280.readAltitude(SEALEVELPRESSURE_HPA)); display.println(" m"); 
  display.print("Humidity: "); display.print(BME280.readHumidity()); display.println(" %"); 
  display.display();
  delay(1000);

  Serial.print("Temperature = ");
  Serial.print(BME280.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");

  Serial.print(BME280.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(BME280.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(BME280.readHumidity());
  Serial.println(" %");

  Serial.println();
  delay(2000);
}