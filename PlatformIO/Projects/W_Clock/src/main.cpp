
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>

// OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Define pin for keypress
#define key_1 1
#define key_2 2

// Define pin for I2C module 2 ---> OLED display
#define I2C_1_SDA 7
#define I2C_1_SCL 15

TwoWire I2C_OLED = TwoWire(1); 

// Declaration for an SSD1306 display connected to I2C1
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, -1); //, -1 if use RTS of ESP32-S3);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Mix_2s";
const char* password = "enterolertcam";

// Variables
bool lastButtonState = HIGH;
int8_t lastDebounceTime = 0;
char send_time[30];

// Constants
const int8_t debounceDelay = 50;  // debounce delay in milliseconds

// Functions
void set_pin_mode();
void oled_startup();
void show_on_display(const char* txt, int8_t x, int8_t y);
bool read_keypress(int8_t key);
void sync_time(void);



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Set pin mode
  set_pin_mode();
    
  // OLED initial
  oled_startup();
  Serial.println();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Sync time
  sync_time();
 
}

void loop() {

  //Serial.println("Button check!");
  // Sync time
  sync_time();
  time_t current_time;
  struct tm* timeinfo;
  time(&current_time);
  timeinfo = localtime(&current_time);
  strftime(send_time, sizeof(send_time), "%Y%m%d_%H%M%S", timeinfo);
  show_on_display(send_time, 20, 0);
  delay(2000);
  
}

void set_pin_mode(){
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

void sync_time(void) {
  const char* ntpServer = "pool.ntp.org";
  configTime(-4*3600, 0, ntpServer); // GMT-4, no DST
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  /* Serial.println("Time synchronized");
  time_t now = time(nullptr);
  Serial.print("Current time: ");
  Serial.println(ctime(&now)); */
}