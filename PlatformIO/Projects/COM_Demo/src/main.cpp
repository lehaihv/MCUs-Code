#include <Arduino.h>

uint8_t value = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // Initialize Serial1 on GPIO 16 (RX) and GPIO 17 (TX)
  Serial.println("Hello, ESP32!");
  delay(1000);
  analogWrite(12, 50); // Set GPIO 2 to a PWM value of 128 (50% duty cycle)
}

void loop() {
  //Serial.println("Hello, ESP32! Looping...");
  delay(50);
  /* value = 20;
  Serial1.println(value);
  Serial.println(value); */
  if (value <25) {
    value++;
    Serial1.println(value);
    Serial.println(value);
  } else {
    value = 0;
  }
  
  /* // Read all available data from the COM port and send all at once
  if (Serial.available() > 0) {
    String receivedData = "";
    while (Serial.available() > 0) {
      receivedData += (char)Serial.read();
      delay(2); // Small delay to allow buffer to fill if data is coming fast
    }
    Serial.print("Received: ");
    Serial.println(receivedData);
    //Serial.print(receivedData); // Echo all received data at once as text
  } */
}

