/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-plot-readings-charts-multiple/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "DFRobot_AS7341.h"
// #include "FS.h"

#define FORMAT_SPIFFS_IF_FAILED true

DFRobot_AS7341 as7341;

// Replace with your network credentials
const char* ssid = "qPCR_Module"; // "REPLACE_WITH_YOUR_SSID";
const char* password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;//30000;

// GPIO where the DS18B20 sensors are connected to
// const int oneWireBus = 4;

// Setup a oneWire instance to communicate with OneWire devices (DS18B20)
// OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
// DallasTemperature sensors(&oneWire);

// Address of each sensor
/*DeviceAddress sensor3 = { 0x28, 0xFF, 0xA0, 0x11, 0x33, 0x17, 0x3, 0x96 };
DeviceAddress sensor1 = { 0x28, 0xFF, 0xB4, 0x6, 0x33, 0x17, 0x3, 0x4B };
DeviceAddress sensor2 = { 0x28, 0xFF, 0x43, 0xF5, 0x32, 0x18, 0x2, 0xA8 };
DeviceAddress sensor4 = { 0x28, 0xFF, 0x11, 0x28, 0x33, 0x18, 0x1, 0x6B };*/

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  DFRobot_AS7341::sModeOneData_t data1;
  DFRobot_AS7341::sModeTwoData_t data2;
  //Start spectrum measurement 
  //Channel mapping mode: 1.eF1F4ClearNIR,2.eF5F8ClearNIR
  as7341.startMeasure(as7341.eF1F4ClearNIR);
  //Read the value of sensor data channel 0~5, under eF1F4ClearNIR
  data1 = as7341.readSpectralDataOne();
  readings["sensor1"] = String(data1.ADF1);
  readings["sensor2"] = String(data1.ADF2);
  readings["sensor3"] = String(data1.ADF3);
  readings["sensor4"] = String(data1.ADF4);
  /*as7341.startMeasure(as7341.eF5F8ClearNIR);
  //Read the value of sensor data channel 0~5, under eF5F8ClearNIR
  data2 = as7341.readSpectralDataTwo();*/

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize SPIFFS
void initSPIFFS() {
  /*if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }*/
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
    /*Serial.println("Erasing SPIFFS...");
    if (SPIFFS.format()) {
      Serial.println("SPIFFS erased successfully.");
    } else {
      Serial.println("Error erasing SPIFFS.");
    }*/
  }
  else{
    Serial.println("SPIFFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
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
   while (as7341.begin() != 0) {
    Serial.println("IIC init failed, please check if the wire connection is correct");
    delay(1000);
  }
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}
