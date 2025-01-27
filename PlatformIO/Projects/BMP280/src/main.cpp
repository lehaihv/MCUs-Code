#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

// Example for demonstrating the TSL2591 library - public domain!

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// define pin for I2C module 1 ---> ADS1115
#define I2C_0_SDA 8
#define I2C_0_SCL 9

// define pin for I2C module 2 ---> OLED display
#define I2C_1_SDA 7
#define I2C_1_SCL 15

TwoWire I2C_BMP280 = TwoWire(0);  // "0", "1" instance of I2C module bus
TwoWire I2C_OLED = TwoWire(1); 

// Declaration for an SSD1306 display connected to I2C1 (SDA1, SCL1 pins)
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, 5); //, -1 if use RTS of ESP32-S3);
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, -1);

Adafruit_BMP280 bmp(&I2C_BMP280); // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void setup() {
  Serial.begin(115200);
  I2C_BMP280.begin(I2C_0_SDA, I2C_0_SCL); 
  I2C_OLED.begin(I2C_1_SDA, I2C_1_SCL); 
  // Oled initial
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64 0x3C (0x78/0x79 for SSD1309 Oled)
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, world!");
  display.display();  

  Serial.println(F("BMP280 test"));
  unsigned status;
  status = bmp.begin(0X76, 0X58);
  //status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.println();

    /////
    display.clearDisplay();
    display.setCursor(40, 0);
    display.println("BMP280"); 
    //display.println();
    display.setCursor(0, 15); display.print(F("Temp: "));display.print(bmp.readTemperature());display.println(" *C");
    display.setCursor(0, 30); display.print(F("Pres: "));display.print(bmp.readPressure());display.println(" Pa");
    display.setCursor(0, 45); display.print(F("Alti: "));display.print(bmp.readAltitude(1013.25));display.println(" m");
    display.display();
    delay(2000);
}