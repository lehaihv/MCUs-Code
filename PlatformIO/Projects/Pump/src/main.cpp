
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Define pin for I2C module 2 ---> OLED display
#define I2C_1_SDA 7
#define I2C_1_SCL 15

TwoWire I2C_OLED = TwoWire(1); 

// Declaration for an SSD1306 display connected to I2C1
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, -1); //, -1 if use RTS of ESP32-S3);

// Define control pin for Pump
#define P1_IN1   17
#define P1_IN2   16
#define P2_IN1   4
#define P3_IN1   5
#define P4_IN1   6
#define PCom_IN2 42 

// Define control pin for Role
#define Valve_P1 18
#define Valve_P2 8
#define Valve_P3 3
#define Valve_P4 46
#define Valve_P5 9

// Define pin for keypress
#define key_1 1
#define key_2 2

// Define constants
#define buffer_time 5000
#define water_time  2100     //2800
#define air_time    1850     //2500

// Variables
int8_t count = 0;
int8_t speed_4ml = 120;  // around 4ml/min
bool lastButtonState = HIGH;
int8_t lastDebounceTime = 0;

// Constants
const int8_t debounceDelay = 50;  // debounce delay in milliseconds

// Functions
void pump(int8_t pump_in1, int8_t pump_in2, int8_t valve, char direction, int8_t speed, char source);
void set_pin_mode();
void oled_startup();
void show_on_display(const char* txt, int8_t x, int8_t y);
bool read_keypress(int8_t key);
void system_clean(int8_t mins);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Set pin mode
  set_pin_mode();
    
  // OLED initial
  oled_startup();
 
}

void loop() {

  // Clean the system
  system_clean(1);
  while(1){}
  // Take the liqud to ready position
  pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
  delay(7500);
  pump(P1_IN1,P1_IN2, Valve_P1, 1, 0, 0);

  // Create 50 small wells
  while(count < 50) { 
    // start cycle
    pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
    delay(water_time); 

    pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 0);
    delay(air_time); 
    
    count++;
  }

  // Create 1 large well
  count = 0;
  while(count < 1) {  
    // start cycle
    pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
    delay(water_time*5); 

    pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 0);
    delay(air_time); 

    count++;
  }
  
  // pump water buffer
  //pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
  //delay(buffer_time);
    
  // pump air buffer
  pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 0);
  delay(buffer_time);
  
  // pump_off();
  pump(P1_IN1,P1_IN2, Valve_P1, 1, 0, 0);

  /* pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
  delay(3000);
  pump(P1_IN1,P1_IN2, Valve_P1, 0, speed_4ml, 1);
  delay(3000);
  pump(P1_IN1,P1_IN2, Valve_P1, 1, 0, 0); */

  Serial.println("Button check!");
  while(1){
    if (read_keypress(key_1)) {
        //Serial.println("Button pressed!");
        pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
        delay(3000);
        pump(P1_IN1,P1_IN2, Valve_P1, 1, 0, 0);
    } else {
        //Serial.println("Button not pressed.");
    }
    //delay(100);  // Adjust delay as needed
    if (read_keypress(key_2)) {
        //Serial.println("Button pressed!");
        pump(P1_IN1,P1_IN2, Valve_P1, 0, speed_4ml, 1);
        delay(3000);
        pump(P1_IN1,P1_IN2, Valve_P1, 0, 0, 0);
    } else {
        //Serial.println("Button not pressed.");
    }
  }
}

// Function to control pump
// int8_t pump_in1, pump_in2, valve: control pins of pump and valve
// char direction: pump direction: 1/Forward; 0/Backward
// int8_t speed: pump flow rate
// char source: 1/liquid; 0/Air
void pump(int8_t pump_in1, int8_t pump_in2, int8_t valve, char direction, int8_t speed, char source){
  (source == 1) ? digitalWrite(valve, HIGH) : digitalWrite(valve, LOW);
  if (direction == 1) {// forward
    show_on_display("Forward", 0, 20);
    analogWrite(pump_in1, speed);
    analogWrite(pump_in2, 0);
  }
  else {
    show_on_display("Backward", 0, 20);
    analogWrite(pump_in1, 0);
    analogWrite(pump_in2, speed);
  } 
}

void set_pin_mode(){
  pinMode(Valve_P1, OUTPUT);
  digitalWrite(Valve_P1, LOW);
  pinMode(key_1, INPUT_PULLUP);
  pinMode(key_2, INPUT_PULLUP);
}

void oled_startup(){
  I2C_OLED.begin(I2C_1_SDA, I2C_1_SCL);  // (I2C_1_SDA, I2C_1_SCL, 100000);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64 0x3C (0x78/0x79 for SSD1309 Oled)
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(100);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  show_on_display("Hello, world!", 15, 10);
  delay(1000);
}

void show_on_display(const char* txt, int8_t x, int8_t y){
  display.clearDisplay();
  display.setCursor(x, y);
  display.println(txt);
  display.display();  
}

bool read_keypress(int8_t key){
  static bool buttonState = HIGH;  // Current stable state
  bool reading = digitalRead(key);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // Reset debounce timer
  }
  // Check if the button state has been stable for the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
    }
  }
  lastButtonState = reading;
  return buttonState == LOW;  // Return true if button is pressed (active LOW)  
}

void system_clean(int8_t mins){
  pump(P1_IN1,P1_IN2, Valve_P1, 1, speed_4ml, 1);
  delay(mins*60000);
  pump(P1_IN1,P1_IN2, Valve_P1, 1, 0, 0);
}