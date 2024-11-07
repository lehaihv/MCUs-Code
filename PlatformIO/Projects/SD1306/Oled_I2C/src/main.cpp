#include <Arduino.h>
#include "Adafruit_VEML7700.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// define pin for I2C module 1 ---> VEML7700_Sensor
#define I2C_0_SDA 8
#define I2C_0_SCL 9

// define pin for I2C module 2 ---> OLED display
#define I2C_1_SDA 7
#define I2C_1_SCL 15

// OLED parameters
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // Change if required 0x78/0x79 for SSD1309 Oled
#define ROTATION 0           // Rotates text on OLED 1=90 degrees, 2=180 degrees

TwoWire I2C_VEML7700 = TwoWire(0);  // "0", "1" instance of I2C module bus
TwoWire I2C_OLED = TwoWire(1); 

Adafruit_VEML7700 veml = Adafruit_VEML7700();


// Define display object
Adafruit_SSD1306 OLed_display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, OLED_RESET);

float lux = 0;

// Initialize VEML7700 sensor
void Init_VEML7700_Sensor() {
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
}

void testdrawstyles() {
  OLed_display.clearDisplay();

  OLed_display.setTextSize(1);             // Normal 1:1 pixel scale
  OLed_display.setTextColor(WHITE);        // Draw white text
  OLed_display.setCursor(0,0);             // Start at top-left corner
  OLed_display.println(F("Hello, world!"));

  OLed_display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  OLed_display.println(3.141592);

  OLed_display.setTextSize(2);             // Draw 2X-scale text
  OLed_display.setTextColor(WHITE);
  OLed_display.print(F("0x")); OLed_display.println(0xDEADBEEF, HEX);

  OLed_display.display();
  delay(2000);
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  I2C_VEML7700.begin(I2C_0_SDA, I2C_0_SCL);  // (I2C_0_SDA, I2C_0_SCL, 100000);
  I2C_OLED.begin(I2C_1_SDA, I2C_1_SCL);  // (I2C_1_SDA, I2C_1_SCL, 100000);
   // Check VEML7700 Sensor is ready 
  if (!veml.begin(&I2C_VEML7700)) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");
  // Init VEML7700 Sensor
  Init_VEML7700_Sensor(); 
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!OLed_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1); // Don't proceed, loop forever
  }
  // Serial.println(F("SSD1306 allocation failed"));
  testdrawstyles();
}

void loop() {
  // put your main code here, to run repeatedly:
  lux = veml.readLux();  // Measure the fluorescence intensity //VEML_LUX_AUTO
  Serial.println(lux);
  delay(1000);
}


