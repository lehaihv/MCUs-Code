#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "Adafruit_VEML7700.h"
#include "MeComAPI.h"
#include "Syringe_Pump_PSD4.h"

// Serial 1 to control TEC module
#define RXD1 16
#define TXD1 17

// Serial 2 to control Syringe Pump
#define PSD4_Addr 0x31            // Address of the PSD4 Pump
#define PSD4_UART 2               // UART port of ESP32 use to communicate with PSD4 Pump, either 0, 1, 2
#define PSD4_UART_Speed 9600      // UART port speed
#define PSD4_RXD 1                // ESP32 Pin number assign for RXD of PSD4_UART Port 
#define PSD4_TXD 2                // ESP32 Pin number assign for TXD of PSD4_UART Port

PSD4_PUMP Main_Pump(PSD4_Addr, PSD4_UART, PSD4_UART_Speed, PSD4_RXD, PSD4_TXD); //Main pump with Addrr = 0x31 and using UART2 RX pin 1 TX pin 2

// Chemicals reservoirs port definitions
int MASTER_MIX_RES = 1;
int WATER_RES = 2;
int AIR_RES = 4; //3;
int REACTOR = 3; //4;

// MeCom definitions
#define SINK_TEMP_EXTERNAL 0
#define SINK_TEMP_FIXED_VALUE 1
#define STATUS_INIT 0
#define STATUS_READY 1
#define STATUS_RUN 2
#define STATUS_ERROR 3
#define STATUS_BOOTLOADER 4
#define STATUS_RESET 5

Adafruit_VEML7700 veml = Adafruit_VEML7700();

// The serial interface to be used by MeCom, either 0, 1, 2
HardwareSerial* ArduinoSerial = &Serial1; 

// Global variables
uint8_t start_qPCR = 0;
uint8_t rep_times = 1;
uint8_t key_debounce = 0;
float lux = 0;
int32_t prev_line = 0;
int32_t PCR_Cycles = 40;
MeParLongFields LutTable_start_stop;
byte pump_busy = 1;
// byte Fan_on = 0;

// Wifi network credentials
const char* ssid = "qPCR_Module"; // The Wifi SSID of the system;
const char* password = "12345678";  // Wifi Password

/* Put IP Address details */
IPAddress local_ip(192,168,1,10);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  //float lux = veml.readLux(VEML_LUX_AUTO); 
  readings["sensor1"] = String(lux);
  readings["sensor2"] = String(lux);
  readings["sensor3"] = String(lux);
  readings["sensor4"] = String(lux);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else{
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  /*WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());*/
  //WiFi.mode(WIFI_AP);
  //WiFi.begin(ssid, password);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  delay(1000);
  /*/ turnoff WiFi when not needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  // turnon WiFi when needed
  WiFi.mode(WIFI_AP);
  WiFi.begin();*/

}

// Check pump not busy
void check_pump_status() {
  pump_busy = 1;
  while (pump_busy){
    delay(200);
    Main_Pump.Get_Pump_Status();
    delay(200);
    if (Main_Pump.Read_from_pump() == 0x60) {
      pump_busy = 0;
      delay(100);
    } 
  }
}

// Initialize VEML7700 sensor
void Init_VEML7700_Sensor() {
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
}

// Initialize PSD4 Pump
void Init_PSD4() {
  //Init pump in high resolution mode
  Main_Pump.Read_from_pump(); // clear buffer
  Main_Pump.Init_Pump(1); 
  check_pump_status();
  // Set motor parameter
  Main_Pump.Set_Motor_Parameters(1, 200, 500, 15, 50); //(2, 50, 1000, 11, 500)
  check_pump_status();
  // Move syringe to Absolute position of 0 
  Main_Pump.Syringe_Move_To_Position(0);
  check_pump_status();
}

// Initialize peripheral pins
void Init_Control_Pin() {
  // Excitation LED control pin initialization
  pinMode(4, OUTPUT);     // sets the digital pin 4 as output to control emission LED 
  // digitalWrite(4, HIGH);  // sets the digital pin 4 high to turn the emission LED on
  // delay(500);
  digitalWrite(4, LOW);  // sets the digital pin 4 low to turn the emission LED off
  // Keypress 1
  pinMode(5, INPUT);     // sets the digital pin 5 as input of Keypress 1
  // Fan control pin
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
}


// Initialize TEC Controller
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
  // Stop the LUTs 
  // MeParLongFields LutTable_start_stop;
  LutTable_start_stop.Value = 1; // a value of 1 means stop LUTs
  MeCom_TEC_Oth_LutTableStop(0, 1, &LutTable_start_stop, MeSet);

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
  LutTable_ID.Value = 0; // a value of 0 means choose sub Table ID number 0
  MeCom_TEC_Oth_LutTableIDSelection(0, 1, &LutTable_ID, MeSet);

  // #define MeCom_TEC_Oth_LutNrOfRepetitions(Address, Inst, Value, Cmd)                 MeCom_ParValuel(Address, 52012, Inst, Value, Cmd)
  // Set "LutNrOfRepetitions" with the desired number of thermal cylcles
  MeParLongFields LutTable_rep;
  LutTable_rep.Value = PCR_Cycles; // a value of 5 means repetition of thermal cylce 5 times
  MeCom_TEC_Oth_LutNrOfRepetitions(0, 1, &LutTable_rep, MeSet);

  // Stop the LUTs 
  // MeParLongFields LutTable_start_stop;
  LutTable_start_stop.Value = 1; // a value of 1 means stop LUTs
  MeCom_TEC_Oth_LutTableStop(0, 1, &LutTable_start_stop, MeSet);
}

// Function to prepare chemicals for qPCR assay 
void qPRC_assay_pre() {
  // Get 100 uL water to the reactor tube
  // Set Water reservoir port to INPUT
  Serial.println("Set Water reservoir port to INPUT");
  Main_Pump.Set_Port_Direction(WATER_RES, 0); 
  check_pump_status();
  // Take in 100 uL of water 
  Serial.println("Take in 100 uL of water ");
  Main_Pump.Syringe_Move_To_Position(24000);
  check_pump_status();
  // Set reactor port to OUTPUT
  Serial.println("Set reactor port to OUTPUT");
  Main_Pump.Set_Port_Direction(REACTOR, 1); 
  check_pump_status();
  // Pump 100 uL of water to reactor tube
  Serial.println("Pump 100 uL of water to reactor tube");
  Main_Pump.Syringe_Move_To_Position(0);
  check_pump_status();

  // Get 50 uL air to the reactor tube
  // Set air reservoir port to INPUT
  Serial.println("Set Air reservoir port to INPUT");
  Main_Pump.Set_Port_Direction(AIR_RES, 0); 
  check_pump_status();
  // Take in 50 uL of air
  Serial.println("Take in 50 uL of air ");
  Main_Pump.Syringe_Move_To_Position(13750);
  check_pump_status();
  // Set reactor port to OUTPUT
  Serial.println("Set reactor port to OUTPUT");
  Main_Pump.Set_Port_Direction(REACTOR, 1); 
  check_pump_status();
  // Pump 50 uL of air to reactor tube
  Serial.println("Pump 50 uL of air to reactor tube");
  Main_Pump.Syringe_Move_To_Position(0);
  check_pump_status();

  // Get 50 uL mastermix, primer and sample to the reactor tube
  // Set master mix reservoir port to INPUT
  Serial.println("Set Master mix reservoir port to INPUT");
  Main_Pump.Set_Port_Direction(MASTER_MIX_RES, 0); 
  check_pump_status();
  // Take in 50 uL of mastermix, primer and sample
  Serial.println("Take in 50 uL of mastermix, primer and sample ");
  Main_Pump.Syringe_Move_To_Position(13750);
  check_pump_status();
  // Set reactor port to OUTPUT
  Serial.println("Set reactor port to OUTPUT");
  Main_Pump.Set_Port_Direction(REACTOR, 1); 
  check_pump_status();
  // Pump 50 uL of mastermix, primer and sample to reactor tube
  Serial.println("Pump 50 uL of mastermix, primer and sample to reactor tube");
  Main_Pump.Syringe_Move_To_Position(0);
  check_pump_status();

  // Get 50 uL air to the reactor tube
  // Set air reservoir port to INPUT
  Serial.println("Set Air reservoir port to INPUT");
  Main_Pump.Set_Port_Direction(AIR_RES, 0); 
  check_pump_status();
  // Take in 50 uL of air
  Serial.println("Take in 50 uL of air ");
  Main_Pump.Syringe_Move_To_Position(13750);
  check_pump_status();
  // Set reactor port to OUTPUT
  Serial.println("Set reactor port to OUTPUT");
  Main_Pump.Set_Port_Direction(REACTOR, 1); 
  check_pump_status();
  // Pump 50 uL of air to reactor tube
  Serial.println("Pump 50 uL of air to reactor tube");
  Main_Pump.Syringe_Move_To_Position(0);
  check_pump_status();

  while (rep_times < 3) {
  // Get 100 uL water to the reactor tube
  // Set Water reservoir port to INPUT
  Serial.println("Set Water reservoir port to INPUT");
  Main_Pump.Set_Port_Direction(WATER_RES, 0); 
  check_pump_status();
  // Take in 100 uL of water 
  Serial.println("Take in 100 uL of water ");
  Main_Pump.Syringe_Move_To_Position(24000);
  check_pump_status();
  // Set reactor port to OUTPUT
  Serial.println("Set reactor port to OUTPUT");
  Main_Pump.Set_Port_Direction(REACTOR, 1); 
  check_pump_status();
  // Pump 100 uL of water to reactor tube
  Serial.println("Pump 100 uL of water to reactor tube");
  Main_Pump.Syringe_Move_To_Position(0);
  check_pump_status();
  rep_times++;
  }  
  rep_times = 1;
  // Set air reservoir port to OUTPUT to move and close reactor port for qPCR
  Serial.println("Set Air reservoir port to OUTPUT");
  Main_Pump.Set_Port_Direction(AIR_RES, 1); 
  check_pump_status();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial1.setTimeout(MEPORT_SET_AND_QUERY_TIMEOUT);
  Serial1.begin(57600, SERIAL_8N1, RXD1, TXD1);  // Serial 1 to communicate with TEC Controller
  // Initialize all peripherals
  Init_Control_Pin();
  // Init TEC controller
  Init_TEC();
  // Init Wifi and SPIFFS
  initWiFi();
  initSPIFFS();
  // Init PSD4 Pump
  // Init_PSD4();
  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.serveStatic("/", SPIFFS, "/");
  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  // Start server
  server.begin();
  // Check VEML7700 Sensor is ready 
  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");
  // Init VEML7700 Sensor
  Init_VEML7700_Sensor(); 
}

void loop() {
  //while (1){}
  // Wait for krypress before transfer sample to qPCR Assay
  /*/ Serial.println("Prepare sample for qPCR Assay ");
  Serial.println("Wait for keypress to prepare sample for qPCR Assay ");
  /*while (key_debounce < 25) {
    if (digitalRead(5) == LOW) {
      key_debounce++;
      delay(10);
    }
    else {key_debounce = 0;}  
  }
  Serial.println("Preparing qPCR Assay ...");
  key_debounce = 0; // Keypress, clear debounce variable
  // Transfer all necessary chemicals
  qPRC_assay_pre();
  Serial.println("qPCR Assay ready");
  
  // Wait for krypress before perform PCR
  Serial.println("Wait for keypress to start qPCR Assay ");
  while (key_debounce < 25) {
    if (digitalRead(5) == LOW) {
      key_debounce++;
      delay(10);
    }
    else {key_debounce = 0;}  
  }
  
  Serial.println("Start qPCR Assay ");
  key_debounce = 0; // Keypress, clear debounce variable*/
  /*digitalWrite(4, HIGH);  // Turn-on excitation LED
  while (1) {
    Serial.print("Fluorescence Intensity, PCR Cycle ");
    Serial.println(rep_times);
    // Serial.println(LuT_line.Value);
    rep_times++;
    delay(1000);  // delay 100ms for light sensor to ready 
    lux = veml.readLux();  // Measure the fluorescence intensity VEML_LUX_AUTO
    Serial.println(lux);
  }*/
  // Get LTU status
  // #define MeCom_TEC_Oth_LutTableStatus(Address, Inst, Value, Cmd)                     MeCom_ParValuel(Address, 52002, Inst, Value, Cmd)
  // Get the status of LuTs, repeat if TEC is not ready or error
  while (start_qPCR == 0) {
    MeParLongFields LuTs_status;
    MeCom_TEC_Oth_LutTableStatus(0, 1, &LuTs_status, MeGet);
    Serial.print("TEC Status Value: ");
    Serial.println( LuTs_status.Value);
    if (LuTs_status.Value == 3) // 3 means LuT is ready
      {
        Serial.println("LuTs Ready");
        // Start the LUTs 
        Init_TEC();
        delay(2000);
        // MeParLongFields LutTable_start_stop;
        LutTable_start_stop.Value = 1; // a value of 1 means start LUTs
        MeCom_TEC_Oth_LutTableStart(0, 1, &LutTable_start_stop, MeSet);
        start_qPCR = 1;
      }
    else
      {
        Serial.println("LuTs Error");
        // Stop the LUTs 
        // MeParLongFields LutTable_start_stop;
        LutTable_start_stop.Value = 1; // a value of 1 means stop LUTs
        MeCom_TEC_Oth_LutTableStop(0, 1, &LutTable_start_stop, MeSet);
        Init_TEC();
        start_qPCR = 0;
        delay(2000);
      }
    delay(500);
  }
  while (start_qPCR == 1 && rep_times < (PCR_Cycles +1)) { //
    digitalWrite(4, HIGH);  // Turn-on excitation LED
    // Measure fluorescence after each thermal cycle, when LuTs is at line 11
    // Get sink temperature, fan on if temp > 40 and off when below 35
    // #define MeCom_TEC_Mon_SinkTemperature(Address, Inst, Value, Cmd)                    MeCom_ParValuef(Address, 1001, Inst, Value, Cmd)
    MeParFloatFields Sink_temp;
    MeCom_TEC_Mon_SinkTemperature(0, 1, &Sink_temp, MeGet);
    if (Sink_temp.Value > 40) {digitalWrite(6, HIGH);} // tun on fan
    if (Sink_temp.Value < 35) {digitalWrite(6, LOW);} // tun off fan
    // get current LuTs execution line
    MeParLongFields LuT_line;
    MeCom_TEC_Oth_LutCurrentTableLine(0, 1, &LuT_line, MeGet);
    // Serial.println(LuT_line.Value);
    if (LuT_line.Value != prev_line) {
      prev_line = LuT_line.Value;
      if (prev_line == 9) { //11
        // digitalWrite(4, HIGH);  // Turn-on excitation LED
        Serial.print("Fluorescence Intensity, PCR Cycle ");
        Serial.println(rep_times);
        // Serial.println(LuT_line.Value);
        rep_times++;
        // delay(100);  // delay 100ms for light sensor to ready 
        lux = veml.readLux();  // Measure the fluorescence intensity //VEML_LUX_AUTO
        Serial.println(lux);
        // Send Events to the client with the Sensor Readings
        events.send("ping",NULL,millis());
        events.send(getSensorReadings().c_str(),"new_readings" ,millis()); 
        // digitalWrite(4, LOW);  // Turn-off excitation LED
      }
    }
    delay(100);
  }
  while (1) {
    digitalWrite(4, LOW);  // Turn-off excitation LED
    Serial.println("Waitting... ");
    start_qPCR = 0;
    rep_times = 1;
    delay(5000);
  }
}
