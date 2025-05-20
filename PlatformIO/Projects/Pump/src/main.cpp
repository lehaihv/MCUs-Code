
#include <Arduino.h>

#define IN1   12
#define IN2   11
#define Valve 13

#define buffer_time 5000
#define water_time 2800
#define air_time 2500

int count = 0;
int speed = 120;  // around 4ml/min



void setup() {
  // put your setup code here, to run once:
  //pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(Valve, OUTPUT);
  //digitalWrite(IN1, LOW);
  analogWrite(IN1, 0);
  digitalWrite(IN2, LOW);
  digitalWrite(Valve, HIGH);
  // analogWrite(14, 250);
  Serial.begin(115200);
  //Serial.setDebugOutput(false);

  ////
  Serial.println("WiFi connected");
  
}

void pump_on() {
  analogWrite(IN1, speed);
  //digitalWrite(IN1, HIGH);
}

void pump_off() {
  analogWrite(IN1, 0);
  //delay(100);
  //digitalWrite(IN1, LOW);
}

void valve_water() {
  digitalWrite(Valve, HIGH);
  //delay(100);
}

void valve_air() {
  digitalWrite(Valve, LOW);
  //delay(100);
}

void loop() {
  // take the liqud to ready position
  valve_water();
  pump_on();
  delay(12000);
  pump_off();    
  //while(1)
  // put your main code here, to run repeatedly:
  // take water to the pump input
 /*  while(1){
    valve_water();
    delay(3000);
    valve_air();
    delay(3000);

  } */
  valve_water();
  pump_on();
  delay(buffer_time);
  // stop pump
  //pump_off();
  // valve air
  valve_air();
  // pump air buffer
  //pump_on();
  delay(buffer_time);
  // stop pump
  //pump_off();
  // valve water
  valve_water();
  

  while(count < 50) { // 15 small wells
    // start cycle
    // pump 20 ul water and 10ul air 
    valve_water();
    //pump_on();
    delay(water_time); // 500us equal to ?ul
    //pump_off();
    // valve air
    valve_air();
    //pump_on();
    delay(air_time); 
    //pump_off();
    count++;
  }

  count = 0;
  while(count < 1) { // 20 large wells 
    // start cycle
    // pump 20 ul water and 10ul air 
    valve_water();
    //pump_on();
    delay(water_time*5); // 500us equal to ?ul
    //pump_off();
    // valve air
    valve_air();
    //pump_on();
    delay(air_time); 
    //pump_off();
    count++;
  }
  
  // pump water buffer
  valve_water();
  //pump_on();
  delay(buffer_time);
  //pump_off();
  // pump air buffer
  valve_air();
  //pump_on();
  delay(buffer_time*2);
  //pump_off();
  valve_water();

  valve_water();
  pump_off();

  while(1){
    // stop pump, valve off
    //valve_water();
    //analogWrite(IN1, 0);
    //pump_off();
  }
  


  /* digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(5000);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  delay(5000); */
}


/* #include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// Replace with your network credentials
const char* ssid = "Mix_2s";
const char* password = "enterolertcam";

// NTP server address
const char* ntpServer = "pool.ntp.org";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure time zone
  configTime(-4*3600, 0, ntpServer); // GMT+1, no DST

  // Wait for time to be synchronized
  while (time(nullptr) == 0) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Time synchronized");
}

void loop() {
  // Get current time
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  // Print the current time
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  delay(1000);
} */