#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  delay(1000);
  analogWrite(12, 50); // Set GPIO 2 to a PWM value of 128 (50% duty cycle)
}

void loop() {
  Serial.println("Hello, ESP32! Looping...");
  delay(1000);

  // Read all available data from the COM port and send all at once
  if (Serial.available() > 0) {
    String receivedData = "";
    while (Serial.available() > 0) {
      receivedData += (char)Serial.read();
      delay(2); // Small delay to allow buffer to fill if data is coming fast
    }
    Serial.print("Received: ");
    Serial.println(receivedData);
    //Serial.print(receivedData); // Echo all received data at once as text
  }
}

