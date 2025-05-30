#include <Arduino.h>
#include "esp_camera.h"
#include "SPI.h"
#include "driver/rtc_io.h"
//#include <ESP_Mail_Client.h>
#include <FS.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <time.h>


// Replace with your network credentials
const char* ssid = "Mix_2s";
const char* password = "enterolertcam";

// Camera module configuration (AI Thinker)
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Camera Stream");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("Camera initialization failed");
    return;
  }

  // Start the streaming server
  startCameraServer();
  Serial.println("Camera Stream Ready");
}

void loop() {
  // Nothing to do in the main loop
}

// Start the camera server
void startCameraServer() {
  // Set up the HTTP server
  WiFiServer server(80);
  server.begin();
  
  while (true) {
    WiFiClient client = server.available();
    if (client) {
      Serial.println("Client connected");
      String response = "";
      response += "HTTP/1.1 200 OK\r\n";
      response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
      response += "\r\n";

      // Send MJPEG stream
      while (client.connected()) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
          Serial.println("Camera capture failed");
          return;
        }

        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n";
        response += "Content-Length: " + String(fb->len) + "\r\n";
        response += "\r\n";
        client.write(response.c_str(), response.length());
        client.write(fb->buf, fb->len);
        client.write("\r\n");

        esp_camera_fb_return(fb);
        delay(100); // Adjust delay for smooth streaming
      }
      client.stop();
      Serial.println("Client disconnected");
    }
  }
}
