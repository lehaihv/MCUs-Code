// Header file 
// For Syringe Pump PSD4
// Name Syringe_Pump_PSD4.h 
// AUthor: Hai LE
// Email: haihv@yahoo.com/hai.le@ufl.edu
// University of Florida 
// July 12 2024


#ifndef Syringe_Pump_PSD4_H 
#define Syringe_Pump_PSD4_H 
#include <Arduino.h> 

// PS4 Pump constant
#define STX   0x02;             // Start byte
#define Sequence_data 0x31;     // Sequence byte
#define ETX  0x03;              // End byte

class PSD4_PUMP { 
private: 
    byte Pump_Addr;
    byte COM_Port;
    int COM_Speed;
    byte RX_Pin;
    byte TX_Pin;

public: 
    PSD4_PUMP(byte, byte, int, byte, byte);
    void Init_Pump(byte operation_mode); 
    void Set_Port_Direction(byte Port_No, byte Port_Dir); 
    void Syringe_Move(byte Syringe_Dir, int No_of_Step);
    void Syringe_Move_To_Position(int Absolute_position); 
    void Syringe_Full_Clean();
    void Get_Pump_Status(); 
    void Set_Motor_Parameters(byte steps_sec_sec, int start_velo, int max_velo, byte speed_code, int stop_velo);
    byte Read_from_pump();
}; 
  
#endif