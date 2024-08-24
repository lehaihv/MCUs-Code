#include <MeComAPI.h>

#define RXD1 16
#define TXD1 17

// MeCom definitions
#define SINK_TEMP_EXTERNAL 0
#define SINK_TEMP_FIXED_VALUE 1

#define STATUS_INIT 0
#define STATUS_READY 1
#define STATUS_RUN 2
#define STATUS_ERROR 3
#define STATUS_BOOTLOADER 4
#define STATUS_RESET 5

// the serial interface to be used by MeCom, either 0, 1, 2
HardwareSerial* ArduinoSerial = &Serial1; 

// variables
uint8_t start_qPCR = 0;
uint8_t rep_times = 0;

void Init_TEC() {
  // get a parameter
  MeParLongFields status;
  uint8_t success = MeCom_COM_DeviceStatus(0, &status, MeGet);
  if (success != 0) {
    // turn the LED on if the TEC controller has encountered an error
    if (status.Value == STATUS_ERROR)
      digitalWrite(LED_BUILTIN, HIGH);
    else
      {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("TEC Ready");
      }
  }
  // #define MeCom_TEC_Ope_OutputStageInputSelection(Address, Inst, Value, Cmd)          MeCom_ParValuel(Address, 2000, Inst, Value, Cmd)
  // Set TEC Chanel "OutputStageInputSelection" to "Temperature Controller" or "Live Current/Voltage"
  // MeCom_TEC_Ope_OutputStageInputSelection(0, 1, 0, MeSet);
  MeParLongFields output_selection;
  output_selection.Value = 2; // a value of 2 means "Temperature Controller"
  MeCom_TEC_Ope_OutputStageInputSelection(0, 1, &output_selection, MeSet);

  // #define MeCom_TEC_Ope_OutputStageEnable(Address, Inst, Value, Cmd)                  MeCom_ParValuel(Address, 2010, Inst, Value, Cmd)
  // Set TEC Chanel "OutputStageEnable" to "Live OFF/ON"
  MeParLongFields output_stage;
  output_stage.Value = 2; // a value of 2 means "Live OFF/ON"
  MeCom_TEC_Ope_OutputStageEnable(0, 1, &output_stage, MeSet);

  // Load CSV LUTs file to TEC Controller
  // Perform using TEC Service Software

  // #define MeCom_TEC_Oth_LutTableIDSelection(Address, Inst, Value, Cmd)                MeCom_ParValuel(Address, 52010, Inst, Value, Cmd)
  // Set "LutTableIDSelection" with the desired Table ID 
  MeParLongFields LutTable_ID;
  LutTable_ID.Value = 0; // a value of 1 means choose sub Table ID number 0
  MeCom_TEC_Oth_LutTableIDSelection(0, 1, &LutTable_ID, MeSet);

  // #define MeCom_TEC_Oth_LutNrOfRepetitions(Address, Inst, Value, Cmd)                 MeCom_ParValuel(Address, 52012, Inst, Value, Cmd)
  // Set "LutNrOfRepetitions" with the desired number of thermal cylcles
  MeParLongFields LutTable_rep;
  LutTable_rep.Value = 5; // a value of 1 means repetition of thermal cylce 5 times
  MeCom_TEC_Oth_LutNrOfRepetitions(0, 1, &LutTable_rep, MeSet);

}

void setup() {
  // serial initialization
  Serial.begin(115200);  //serialo to connect IDE and debug
  Serial1.setTimeout(MEPORT_SET_AND_QUERY_TIMEOUT);
  Serial1.begin(57600, SERIAL_8N1, RXD1, TXD1);  //serial1 to communicate with TEC

  // LED initialization
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Init TEC controller
  Init_TEC();
  // set a parameter
  /*MeParLongFields sink_temp;
  sink_temp.Value = SINK_TEMP_EXTERNAL;
  MeCom_TEC_Sin_SinkTemperatureSelection(0, 1, &sink_temp, MeSet);*/
}

void loop() {
  // Get LTU status
  // #define MeCom_TEC_Oth_LutTableStatus(Address, Inst, Value, Cmd)                     MeCom_ParValuel(Address, 52002, Inst, Value, Cmd)
  // Get the status of LuTs
  MeParLongFields LuTs_status;
  MeCom_TEC_Oth_LutTableStatus(0, 1, &LuTs_status, MeGet);
  Serial.println(LuTs_status.Value);
  if (LuTs_status.Value == 3) // 3 means LuT is ready
    {
      Serial.println("LuTs Ready");
      // Start the LUTs 
      MeParLongFields LutTable_start_stop;
      LutTable_start_stop.Value = 1; // a value of 1 means start LUTs
      MeCom_TEC_Oth_LutTableStart(0, 1, &LutTable_start_stop, MeSet);
      start_qPCR = 1;
      }
  else
    {
      Serial.println("LuTs Error");
      start_qPCR = 0;
      }
  while (start_qPCR){
    // Measure fluorescence after each thermal cycle, when LuTs is at line 11
    // get current LuTs execution line
    MeParLongFields LuT_line;
    MeCom_TEC_Oth_LutCurrentTableLine(0, 1, &LuT_line, MeGet);
    if (LuT_line.Value == 11) {
      Serial.print("Measure happen ");
      Serial.println(rep_times);
      Serial.println(LuT_line.Value);
      rep_times++;
    }
    delay(1000);
  }

  
}
