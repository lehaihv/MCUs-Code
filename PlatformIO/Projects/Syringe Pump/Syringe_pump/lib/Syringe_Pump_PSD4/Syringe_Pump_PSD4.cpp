// C++ program to include the 
// custom header File for syringe Pump PSD4
// AUthor: Hai LE
// University of Florida 
// July 12 2024

// Include statement to include 
// custom header file 
#include "Syringe_Pump_PSD4.h" 
#include <HardwareSerial.h>

// Constants and variables
HardwareSerial* Pump_UART = NULL;
byte CKSUM;
const int REC_BUFFER_SIZE = 50;
byte inByte[REC_BUFFER_SIZE];

// Function to calculate checksum byte
byte Cal_Checksum(byte buffer[], byte length_buffer)
{
	//unsigned char array[6] = {0x03, 0x31, 0x31, 0x5A, 0x52, 0x03};
    //unsigned char checksum = 0;
	CKSUM = 0;

    // Calculate the checksum bit-by-bit
    for (int bit = 0; bit < 8; bit++) {
        byte bitSum = 0;
        for (int i = 0; i < length_buffer; i++) {
            // Check the value of the current bit in each array element
            if (buffer[i] & (1 << bit)) {
                bitSum++;
            }
        }
        // Set the checksum bit based on the sum being odd or even
        if (bitSum % 2 != 0) {
            CKSUM |= (1 << bit);
        }
    }

	return CKSUM;
}


/// @brief 
/// @param Pump_Adrr 
/// @param COM_Port 
PSD4_PUMP::PSD4_PUMP(byte Pump_Addr, byte COM_Port, int COM_Speed, byte RX_Pin, byte TX_Pin) 
{ 
	this->COM_Port = COM_Port;
	this->Pump_Addr = Pump_Addr; 
	this->COM_Speed = COM_Speed;
	this->RX_Pin = RX_Pin;
	this->TX_Pin = TX_Pin;
	if (COM_Port == 0) Pump_UART = &Serial;
	else if (COM_Port == 1) Pump_UART = &Serial1;
	else Pump_UART = &Serial2;
	Pump_UART->begin(COM_Speed, SERIAL_8N1, RX_Pin, TX_Pin);
} 

// Function to initiallize the Pump 1st time after it is power on / reset
// operation_mode: 1 (High resolution) or (0) (Standard)
void PSD4_PUMP::Init_Pump(byte operation_mode) 
{ 
	byte temp[9];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	temp[3] = 'Z';  //0x5A; // "Z" initializes the syringe to the home position 
	//and sets valve output position to the right side of the PSD/4 (as viewed from the front of the PSD/4).
	// temp[4], [5]: N1/0: high resolution/standard mode
	temp[4] = 'N';  //0x4E; // "N" set pump mode
	temp[5] = operation_mode + 0x30;
	temp[6] = 'R';  //0x52; // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[7] = ETX; 
	// Calculate Checksum 
	temp[8] = Cal_Checksum(temp, 8);
	//Serial.println(COM_Port);
	Pump_UART->write(temp, 9);
	/*for(int i = 0; i < 7; i++)
		Serial.println(temp[i], HEX);*/
} 

// Function to set Port number (1 to 8) of Valve to 
// OUTPUT O-0x4F (Port_Dir = 1) or INPUT I-0x49 (Port_Dir = 0)
void PSD4_PUMP::Set_Port_Direction(byte Port_No, byte Port_Dir) 
{ 
	byte temp[8];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	temp[3] = (Port_Dir == 0) ? 'O' : 'I'; //0x4F : 0x49; // Direction of Port 
	temp[4] = Port_No + 0x30; // Port number; from 1 to 8, to be set direction 
	temp[5] = 'R';  //0x52; // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[6] = ETX; 
	// Calculate Checksum 
	temp[7] = Cal_Checksum(temp, 7);
	Pump_UART->write(temp, 8); 
} 

// Function to move syringe up/down X step
// Syringe_Dir: 1 (UP) or (0) (DOWN)
// No_of_Step: number of steps that syringe will move
void PSD4_PUMP::Syringe_Move(byte Syringe_Dir, int No_of_Step)
{ 
	byte temp[12];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	if (Syringe_Dir == 0) temp[3] = 'P';  //0x50; //"P-0x50" -- Syringe goes down
	else if (Syringe_Dir == 1) temp[3] = 'D';  //0x44; //"D-0x44" -- Syringe goes up
	// 5 bytes from temp[4] to temp[8]: number of steps that syringe will move: 0-300/0-24000 in standard/high resolution mode
	for (int i = 8; i >= 4; i--) {
		temp[i] = (No_of_Step % 10) + 0x30;
		No_of_Step /= 10;
	}
	temp[9] = 'R';  //0x52; // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[10] = ETX; 
	// Calculate Checksum 
	temp[11] = Cal_Checksum(temp, 11);
	Pump_UART->write(temp, 12); 
} 

void PSD4_PUMP::Syringe_Full_Clean() 
{ 	

	
}

// Function to set parameters of motor
// steps_sec_sec: 1 - 20 
// start_velo: 50 - 1000
// max_velo: 2 - 10000
// speed_code: 1 - 40
// stop_velo: 50 - 2700
void PSD4_PUMP::Set_Motor_Parameters(byte steps_sec_sec, int start_velo, int max_velo, byte speed_code, int stop_velo) 
{ 	
	byte temp[28];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	// L-steps_sec_sec: 1- 20
	temp[3] = 'L'; //0x4C; //"L" -- Set Acceleration
	temp[4] = (steps_sec_sec / 10) + 0x30; //Acceleration code 
	temp[5] = (steps_sec_sec % 10) + 0x30;
	// v-start_velo: 50 - 1000
	temp[6] = 'v';  //0x76; //"v" -- Set start velocity
	// 4 bytes from temp[7] to temp[10]: start_velo: 50 - 1000
	for (int i = 10; i >= 7; i--) {
		temp[i] = (start_velo % 10) + 0x30;
		start_velo /= 10;
	}
	// V-max_velo: 2 - 10000
	temp[11] = 'V';  //0x56; //"V" -- Set max velocity
	// 5 bytes from temp[12] to temp[16]: max_velo: 2 - 10000
	for (int i = 16; i >= 12; i--) {
		temp[i] = (max_velo % 10) + 0x30;
		max_velo /= 10;
	} 
	// speed_code: 1 - 40
	temp[17] = 'S';  //0x53; //"S" -- set speed
	temp[18] = (speed_code / 10) + 0x30; //speed code
	temp[19] = (speed_code % 10) + 0x30;
	// stop_velo: 50 - 2700
	temp[20] = 'c';  //0x63; //"c" -- set stop velocity
	// 4 bytes from temp[21] to temp[24]: max_velo: 2 - 2700
	for (int i = 24; i >= 21; i--) {
		temp[i] = (stop_velo % 10) + 0x30;
		stop_velo /= 10;
	}  
	temp[25] = 'R';  //0x52; // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[26] = ETX; 
	// Calculate Checksum 
	temp[27] = Cal_Checksum(temp, 27);
	//Serial.println(COM_Port);
	Pump_UART->write(temp, 28); 
	
}

void PSD4_PUMP::Read_from_pump() 
{
	if (Pump_UART->available()) {
		/// read the incoming bytes:
		int rlen = Pump_UART->readBytes(inByte, REC_BUFFER_SIZE);
		Serial.println(rlen);
		/// prints the received data
		Serial.println("I received: ");
		for(int i = 0; i < rlen; i++)
			Serial.println(inByte[i], HEX); // Serial.print(int(inByte[i]));
		/// if (inByte[0] == 0x02) Serial.println("Good start");
		Serial.println();
	}
}