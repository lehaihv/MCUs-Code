#include <Arduino.h>

#define IN1   12
#define IN2   11
#define Valve 13

int count = 0;
int speed = 80;

void setup() {
  // put your setup code here, to run once:
  //pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(Valve, OUTPUT);
  // digitalWrite(IN1, LOW);
  analogWrite(IN1, 0);
  digitalWrite(IN2, LOW);
  digitalWrite(Valve, LOW);
  // analogWrite(14, 250);
  Serial.begin(115200);
  //Serial.setDebugOutput(false);

  ////
  Serial.println("WiFi connected");
  
}

void pump_on() {
  analogWrite(IN1, speed);
}

void pump_off() {
  //analogWrite(IN1, 0);
  //delay(100);
}

void valve_water() {
  digitalWrite(Valve, LOW);
  //delay(100);
}

void valve_air() {
  digitalWrite(Valve, HIGH);
  //delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  // take water to the pump input
  valve_water();
  pump_on();
  delay(10000);
  // stop pump
  pump_off();
  // valve air
  valve_air();
  // pump air buffer
  pump_on();
  delay(8000);
  // stop pump
  pump_off();
  // valve water
  valve_water();
  

  while(count < 45) {
    // start cycle
    // pump 20 ul water and 10ul air 
    valve_water();
    pump_on();
    delay(1000); // 500us equal to ?ul
    pump_off();
    // valve air
    valve_air();
    pump_on();
    delay(1000); 
    pump_off();
    count++;
  }
  
  // pump water buffer
  valve_water();
  pump_on();
  delay(5000);
  pump_off();
  // pump air buffer
  valve_air();
  pump_on();
  delay(10000);
  pump_off();
  valve_water();

  valve_water();
  analogWrite(IN1, 0);

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