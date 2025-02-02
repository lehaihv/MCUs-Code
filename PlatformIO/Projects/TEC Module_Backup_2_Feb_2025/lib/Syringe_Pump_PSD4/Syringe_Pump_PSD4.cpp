// C++ program to include the 
// Custom header file for Syringe Pump PSD4
// AUthor: Hai LE 
// Email: haihv@yahoo.com/hai.le@ufl.edu
// University of Florida 
// July 12 2024

// Include statement to include 
// custom header file 
#include "Syringe_Pump_PSD4.h" 
#include <HardwareSerial.h>

// Constants and variables
HardwareSerial* Pump_UART = NULL;  // UART Port to communicate with PSD4 Pump
byte CKSUM;						   // Checksum byte
const int REC_BUFFER_SIZE = 50;    // Size of UART buffer to receive data from PSD4 pump
byte inByte[REC_BUFFER_SIZE];	   // UART buffer to receive data from PSD4 pump

// Function to calculate checksum byte
byte Cal_Checksum(byte buffer[], byte length_buffer)
{
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


// Function to initialize the PSD4 Pump
// Pump_Adrr: Address of PSD4 Pump
// COM_Port: UART port number of MCU to comunicate with PSD4 Pump
// COM_Speed: Baud rate of UART communication
// byte RX_Pin: RXD pin of UART
// byte TX_Pin: TXD pin of UART
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
// Operation_mode: 1 (High resolution) or (0) (Standard)
void PSD4_PUMP::Init_Pump(byte operation_mode) 
{ 
	byte temp[9];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	temp[3] = 'Z';  // "Z" initializes the syringe to the home position 
					//and sets valve output position to the right side of the PSD/4 (as viewed from the front of the PSD/4).
	// temp[4], [5]: N1/0: high resolution/standard mode
	temp[4] = 'N';  // "N" set pump mode
	temp[5] = operation_mode + 0x30;
	temp[6] = 'R';  // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[7] = ETX; 
	// Calculate Checksum 
	temp[8] = Cal_Checksum(temp, 8);
	Pump_UART->write(temp, 9);
} 

// Function to set Port number (1 to 8) of Valve to 
// OUTPUT "O"-0x4F (Port_Dir = 1) or INPUT "I"-0x49 (Port_Dir = 0)
void PSD4_PUMP::Set_Port_Direction(byte Port_No, byte Port_Dir) 
{ 
	byte temp[8];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	temp[3] = (Port_Dir == 1) ? 'O' : 'I';  // Direction of Port 
	temp[4] = Port_No + 0x30; // Port number; from 1 to 8, to be set direction 
	temp[5] = 'R';  // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[6] = ETX; 
	// Calculate Checksum 
	temp[7] = Cal_Checksum(temp, 7);
	Pump_UART->write(temp, 8); 
} 

// Function to move syringe up/down X step from current position
// Syringe_Dir: 1 (UP) or (0) (DOWN)
// No_of_Step: number of steps that syringe will move
void PSD4_PUMP::Syringe_Move(byte Syringe_Dir, int No_of_Step)
{ 
	byte temp[12];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	if (Syringe_Dir == 0) temp[3] = 'P';  //"P-0x50" -- Syringe goes down
	else if (Syringe_Dir == 1) temp[3] = 'D';  //"D-0x44" -- Syringe goes up
	// 5 bytes from temp[4] to temp[8]: number of steps that syringe will move: 0-300/0-24000 in standard/high resolution mode
	for (int i = 8; i >= 4; i--) {
		temp[i] = (No_of_Step % 10) + 0x30;
		No_of_Step /= 10;
	}
	temp[9] = 'R';  // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[10] = ETX; 
	// Calculate Checksum 
	temp[11] = Cal_Checksum(temp, 11);
	Pump_UART->write(temp, 12); 
} 

// Function to move syringe to absolute position
// Absolute_position: position to move syringe to 0-300/0-24000 in standard/high resolution mode
void PSD4_PUMP::Syringe_Move_To_Position(int Absolute_position) 
{
	byte temp[12];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	temp[3] = 'A';  //"A" -- Syringe goes to absolute position
	// 5 bytes from temp[4] to temp[8]: absolute position that syringe will move to
	// 0-300/0-24000 in standard/high resolution mode
	for (int i = 8; i >= 4; i--) {
		temp[i] = (Absolute_position % 10) + 0x30;
		Absolute_position /= 10;
	}
	temp[9] = 'R';  // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[10] = ETX; 
	// Calculate Checksum 
	temp[11] = Cal_Checksum(temp, 11);
	Pump_UART->write(temp, 12); 
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
	temp[3] = 'L';  //"L" -- Set Acceleration
	temp[4] = (steps_sec_sec / 10) + 0x30; //Acceleration code 
	temp[5] = (steps_sec_sec % 10) + 0x30;
	// v-start_velo: 50 - 1000
	temp[6] = 'v';  //"v" -- Set start velocity
	// 4 bytes from temp[7] to temp[10]: start_velo: 50 - 1000
	for (int i = 10; i >= 7; i--) {
		temp[i] = (start_velo % 10) + 0x30;
		start_velo /= 10;
	}
	// V-max_velo: 2 - 10000
	temp[11] = 'V';  //"V" -- Set max velocity
	// 5 bytes from temp[12] to temp[16]: max_velo: 2 - 10000
	for (int i = 16; i >= 12; i--) {
		temp[i] = (max_velo % 10) + 0x30;
		max_velo /= 10;
	} 
	// speed_code: 1 - 40
	temp[17] = 'S';  //"S" -- set speed
	temp[18] = (speed_code / 10) + 0x30; //speed code
	temp[19] = (speed_code % 10) + 0x30;
	// stop_velo: 50 - 2700
	temp[20] = 'c';  //"c" -- set stop velocity
	// 4 bytes from temp[21] to temp[24]: max_velo: 2 - 2700
	for (int i = 24; i >= 21; i--) {
		temp[i] = (stop_velo % 10) + 0x30;
		stop_velo /= 10;
	}  
	temp[25] = 'R';  // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[26] = ETX; 
	// Calculate Checksum 
	temp[27] = Cal_Checksum(temp, 27);
	Pump_UART->write(temp, 28); 
	
}

// Function to perform a wash of the pump
// SHould be developed based on specific need 
void PSD4_PUMP::Syringe_Full_Clean() 
{ 	

	
}

// Function to query current status of PSD4 pump
void PSD4_PUMP::Get_Pump_Status()
{
	byte temp[6];
	temp[0] = STX;
	temp[1] = Pump_Addr;
	temp[2] = Sequence_data;
	// Data to send
	temp[3] = 'Q';  //"Q" -- Get the status of PSD4 pump
	//temp[4] = 'R';  // "R" executes the first unexecuted command in the command buffer.
	// EXT byte
	temp[4] = ETX; 
	// Calculate Checksum 
	temp[5] = Cal_Checksum(temp, 5);
	Pump_UART->write(temp, 6); 

}

// Function to read the return data from the pump
// Data including number of received bytes and all data bytes reponse from pump
// Data bytes show in HEX but can be modified if needed 
byte PSD4_PUMP::Read_from_pump() 
{
	if (Pump_UART->available()) {
		/// read the incoming bytes:
		int rlen = Pump_UART->readBytes(inByte, REC_BUFFER_SIZE);
		/// prints the received data
		Serial.print("Received from pump: ");
		Serial.print(rlen);
		Serial.print(" bytes: ");
		for(int i = 0; i < rlen; i++)
			{
			Serial.print(inByte[i], HEX);
			Serial.print(" ");
			}
		Serial.println();
		//return inByte[3];
	}	
	return inByte[3];
}
