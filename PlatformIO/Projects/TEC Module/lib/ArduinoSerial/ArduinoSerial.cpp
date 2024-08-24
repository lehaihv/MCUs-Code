#ifdef ARDUINO
#include "../MeComAPI/MePort.h"
#include "ArduinoSerial.h"

void ArduinoSerial_recvData(void)
{
    int8_t RcvBuf[MEPORT_MAX_RX_BUF_SIZE + 1];
    size_t nread = ArduinoSerial->readBytesUntil('\n', (char*)RcvBuf, MEPORT_MAX_RX_BUF_SIZE);
    if (nread > 0) {
        RcvBuf[nread] = 0;
        MePort_ReceiveByte((int8_t*)RcvBuf);
    }
}

void ArduinoSerial_Send(char *buffer)
{
    ArduinoSerial->write(buffer);
    ArduinoSerial->flush();
}
#endif