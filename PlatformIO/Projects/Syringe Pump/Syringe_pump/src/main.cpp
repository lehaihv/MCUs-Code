#include <Arduino.h>
#include "Syringe_Pump_PSD4.h"

// Serial 1 to control TEC module
#define RXD1 16
#define TXD1 17

// Serial 2 
//#define RXD2 1
//#define TXD2 2

// Serial 2 to control Syringe Pump
#define PSD4_Addr 0x31            // Address of the PSD4 Pump
#define PSD4_UART 2               // UART port use to communicate with PSD4 Pump, either 0, 1, 2
#define PSD4_UART_Speed 9600      // UART port speed
#define PSD4_RXD 1                // Pin number assign for RXD of PSD4_UART Port 
#define PSD4_TXD 2                // Pin number assign for TXD of PSD4_UART Port

PSD4_PUMP Main_Pump(PSD4_Addr, PSD4_UART, PSD4_UART_Speed, PSD4_RXD, PSD4_TXD); //Main pump with Addrr = 0x31 and using UART2 RX pin 1 TX pin 2

//////

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  // Serial1.setTimeout(MEPORT_SET_AND_QUERY_TIMEOUT);           
  // Serial1.begin(57600, SERIAL_8N1, RXD1, TXD1);  // Serial 1 to communicate with TEC Controller
  //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);   // Serial 2 to communicate with Syringe Pump
  // Excitation LED control pin initialization
  pinMode(4, OUTPUT);     // sets the digital pin 4 as output to control emission LED 
  digitalWrite(4, HIGH);  // sets the digital pin 4 off to turn the emission LED off
  // Initiallize the Pump
  // Main_Pump.Init_Pump();
  /*Main_Pump.Read_from_pump();

  Main_Pump.Init_Pump(1); //Init pump in high resolution mode
  delay(500);
  Main_Pump.Read_from_pump();
  delay(100);
  Main_Pump.Syringe_Move(0, 20432);
  delay(500);
  //Serial2.println("/1ZR\r"); //\rRZ1/"); //
  Serial.println("pump move");
  Main_Pump.Read_from_pump();*/
  //Serial2.println("/1ZR\r");
  //char init_pump[5] = {0x2F, 0x31, 0x5A, 0x52, 0x0D};
  /*char init_pump[5] = {0x2F, 0x31, 0x5A, 0x52, 0x0D,};
  Serial2.write(init_pump, 4);
  Serial.println("pump move");
  delay(500);
  char move_pump[8] = {0x2F, 0x31, 0x41, 0x30, 0x00, 0x00, 0x52, 0x0D};
  Serial2.write(move_pump, 7);
  Serial.println("pump move");*/
  /*char init_pump[9] = {'/', '1', 'A', '3', '0', '0', 'R', '\r', '\n'};
  Serial2.write(init_pump, 9);
  Serial.println("pump move");*/
}

void loop() {
  // put your main code here, to run repeatedly:
  // extra step to clear out MCU buffer
  Main_Pump.Read_from_pump();

  Main_Pump.Init_Pump(1); //Init pump in high resolution mode
  delay(2000);
  Main_Pump.Read_from_pump();
  delay(1000);
  Serial.println("pump move");
  Main_Pump.Syringe_Move(0, 20432);
  delay(2000);
  Main_Pump.Set_Port_Direction(1, 1); // Set Port 1 to OUTPUT
  delay(500);
  Main_Pump.Read_from_pump();
  delay(100);
  Main_Pump.Set_Port_Direction(3, 0);  // Set Port 3 to INPUT
  Main_Pump.Syringe_Move(0, 20432);
  delay(500);
  Main_Pump.Read_from_pump();
  delay(100);
  Main_Pump.Set_Motor_Parameters(2, 50, 1000, 11, 500);
  delay(500);
  Main_Pump.Read_from_pump();
  delay(100);
  
}
