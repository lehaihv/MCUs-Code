#ifndef ARDUINOSERIAL_H
#define ARDUINOSERIAL_H

#ifdef ARDUINO
#include <HardwareSerial.h>

extern void ArduinoSerial_recvData(void);
extern void ArduinoSerial_Send(char* buffer);
extern HardwareSerial* ArduinoSerial;
#endif

#endif // !ARDUINOSERIAL_H