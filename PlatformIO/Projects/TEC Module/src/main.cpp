#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "Adafruit_VEML7700.h"
#include "MeComAPI.h"

// Serial 1 to control TEC module
#define RXD1 16
#define TXD1 17

// Serial 2 to control Syringe Pump
/*#define RXD2 1
#define TXD2 2*/

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

// the serial interface to be used by MeCom, either 0, 1, 2
HardwareSerial* ArduinoSerial = &Serial1; 

// PS4 Pump constant
/*const char Pump_add = 0x31;
const char STX = 0x02;
const char Sequence_data = 0x31;
const char ETX = 0x03;*/


// variables
uint8_t start_qPCR = 0;
// uint8_t serve_update_data = 0;
uint8_t rep_times = 0;
float lux = 0;
int32_t prev_line = 0;
int32_t PCR_Cycles = 3;
MeParLongFields LutTable_start_stop;

// Replace with your network credentials
const char* ssid = "qPCR_Module"; // "REPLACE_WITH_YOUR_SSID";
const char* password = "12345678";

// const char* ssid = "ESP32";  // Enter SSID here
// const char* password = "87654321";  //Enter Password here

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

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;//30000;

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  //float lux = veml.readLux(VEML_LUX_AUTO); // sensors.requestTemperatures();
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

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial1.setTimeout(MEPORT_SET_AND_QUERY_TIMEOUT);
  Serial1.begin(57600, SERIAL_8N1, RXD1, TXD1);  // Serial 1 to communicate with TEC Controller
  //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // Serial 2 to communicate with Syringe Pump
  // Excitation LED control pin initialization
  pinMode(4, OUTPUT);     // sets the digital pin 4 as output to control emission LED 
  digitalWrite(4, HIGH);  // sets the digital pin 4 off to turn the emission LED on
  delay(500);
  digitalWrite(4, LOW);  // sets the digital pin 4 off to turn the emission LED off
  // Init TEC controller
  Init_TEC();
  initWiFi();
  initSPIFFS();
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
  if (!veml.begin()) {
    Serial.println("Sensor not found");
    while (1);
  }
  Serial.println("Sensor found");
}

void loop() {
  /*if (serve_update_data == 1) {  //((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
    serve_update_data = 0;
    //float lux = veml.readLux(VEML_LUX_AUTO); // sensors.requestTemperatures();
    //Serial.println(lux);
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
        // Init_TEC();
        // MeParLongFields LutTable_start_stop;
        LutTable_start_stop.Value = 1; // a value of 1 means stop LUTs
        MeCom_TEC_Oth_LutTableStop(0, 1, &LutTable_start_stop, MeSet);
        start_qPCR = 0;
        }
    delay(500);
  }
  while (start_qPCR == 1 && rep_times < PCR_Cycles) { //
    // Measure fluorescence after each thermal cycle, when LuTs is at line 11
    // get current LuTs execution line
    MeParLongFields LuT_line;
    MeCom_TEC_Oth_LutCurrentTableLine(0, 1, &LuT_line, MeGet);
    // Serial.println(LuT_line.Value);
    if (LuT_line.Value != prev_line) {
      prev_line = LuT_line.Value;
      if (prev_line == 11) { //11
        digitalWrite(4, HIGH);  // Turn-on excitation LED
        Serial.print("Measure happen ");
        Serial.println(rep_times);
        Serial.println(LuT_line.Value);
        rep_times++;
        delay(100);  // delay 100ms for light sensor to ready 
        lux = veml.readLux(VEML_LUX_AUTO); // sensors.requestTemperatures();
        Serial.println(lux);
        // serve_update_data = 1;
        // Send Events to the client with the Sensor Readings
        events.send("ping",NULL,millis());
        events.send(getSensorReadings().c_str(),"new_readings" ,millis());
        digitalWrite(4, LOW);  // Turn-off excitation LED
      }
    }
    delay(100);
  }
  while (1) {
    Serial.println("Waitting... ");
    start_qPCR = 0;
    delay(2000);
  }
}
