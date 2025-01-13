#include <Arduino.h>
#include "Adafruit_VEML7700.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <ESP32TimerInterrupt.h>
//#include "esp_timer.h"

//esp_timer timer = 0;

// #define PIN_INPUT 0
//#define PIN_OUTPUT 4
char string[16];
uint32_t f;

/* unsigned long halfPeriod = 100; // 32 bit data to hold frequency = halfPeriod x 2
unsigned long currentMillis; // 32 bit value for time comparison
unsigned long elapsedMillis;  // 32 bit value for time comparison
unsigned long previousMillis; // 32 bit value for time comparison
 */
// Timer0 Configuration Pointer (Handle)
hw_timer_t *Timer0_Cfg = NULL;

// define pin for I2C module 1 ---> VEML7700_Sensor
#define I2C_0_SDA 8
#define I2C_0_SCL 9

// define pin for I2C module 2 ---> OLED display
#define I2C_1_SDA 7
#define I2C_1_SCL 15

// OLED parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

TwoWire I2C_VEML7700 = TwoWire(0);  // "0", "1" instance of I2C module bus
TwoWire I2C_OLED = TwoWire(1); 

Adafruit_VEML7700 veml = Adafruit_VEML7700();


// Declaration for an SSD1306 display connected to I2C1 (SDA1, SCL1 pins)
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, 5); //, -1 if use RTS of ESP32-S3);
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, -1);

float lux = 0;
double Setpoint, Input, Output;

// Initialize VEML7700 sensor
void Init_VEML7700_Sensor() {
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
}

void IRAM_ATTR Timer0_ISR() { 

        // Your code to execute every 100ms
        //digitalWrite(4, !digitalRead(4));

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
  // Excitation LED control pin initialization
  pinMode(4, OUTPUT);     // sets the digital pin 4 as output to control emission LED 
  // digitalWrite(4, HIGH);  // sets the digital pin 4 high to turn the emission LED on
  // delay(500);
  // digitalWrite(4, LOW);  // sets the digital pin 4 low to turn the emission LED off
  // analogWrite(PIN_OUTPUT, 10);
  // Set up timer
  //timer.attachInterrupt(timerISR, 10, true); // Attach interrupt to timerISR, frequency 10Hz, true for microsecond precision
  //timerBegin(1, 80, true); // 80 is the compare match register value for 100ms
  //timerAttachInterrupt(0, timerISR, true); // Attach the ISR
  // Configure Timer0 Interrupt 10000/second
  /* Timer0_Cfg = timerBegin(0, 2400, true);  //0: timer 0 (0-3); 400: prescaler; true/false: counter should count up (true) or down (false) 
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);  // attach timer to an ISR
  timerAlarmWrite(Timer0_Cfg, 1000, true);  // specify the counter value in which the timer interrupt should be generated: 10000 -->100ms if clock is 40MHz/400 = 100kHz
  timerAlarmEnable(Timer0_Cfg);  // enable timer interrupt
  //read what it thinks it now is */
  f = getCpuFrequencyMhz();
  sprintf(string, "CPU Freq: %i", f);
  Serial.println(string);
}

void loop() {
  
  /* // see if time to change output level
  currentMillis = millis(); // capture the current 'time'
  elapsedMillis = currentMillis - previousMillis; // how much 'time' has passed since last capture
  if (elapsedMillis >= halfPeriod)
  { // enough has passed: do something
    previousMillis = previousMillis + halfPeriod; // set up for next level change
    //PIND = 0b00000100; // flips D2 from Hi to Lo, or Lo to Hi
    digitalWrite(4, !digitalRead(4));
  } */

  // put your main code here, to run repeatedly:
  //digitalWrite(4, HIGH);
  Output = 150;
  analogWrite(4, Output);
  delay(500);
  lux = veml.readLux();  // Measure the fluorescence intensity //VEML_LUX_AUTO
  Serial.print("Value: "); 
  Serial.println(lux); //*1000
  analogWrite(4, 0);

  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Values");  // "Differential: "); 
  display.println();
  display.print(lux);
  display.display(); 


  //digitalWrite(4, LOW);
  delay(1000);

  //pinMode(4, OUTPUT);     // sets the digital pin 4 as output to control emission LED 
   
}


