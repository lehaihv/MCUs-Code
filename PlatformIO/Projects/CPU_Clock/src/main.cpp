#include <Arduino.h>

char string[16];
uint32_t f;

void setup() {
  Serial.begin(115200);
  f = getCpuFrequencyMhz();
  sprintf(string, "CPU Freq: %i", f);
  Serial.println(string);
  //set to 40MHz
  setCpuFrequencyMhz(40);
}

void loop() {
  //read what it thinks it now is
  f = getCpuFrequencyMhz();
  sprintf(string, "CPU Freq: %i", f);
  Serial.println(string);
  delay (1000);
}