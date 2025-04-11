#include <Arduino.h>
#include <Adafruit_NeoPixel.h> 

#define PIN 38
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 1000

// Our task: blink an LED
void toggleLED(void *parameter) {
  while(1) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    pixels.show();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Task 1");
  }
}

void setup() {

  Serial.begin(115200);
  // Configure pixels
  pixels.begin();
  pixels.setBrightness(3);
  pixels.clear();

  // Task to run forever
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              toggleLED,    // Function to be called
              "Toggle LED", // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL,         // Task handle
              0);           // Run on one core for demo purposes (ESP32 only) 

  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() in
  // main after setting up your tasks.
}

void loop() {
  // Do nothing
  
  // setup() and loop() run in their own task with priority 1 in core 1
  // on ESP32
}