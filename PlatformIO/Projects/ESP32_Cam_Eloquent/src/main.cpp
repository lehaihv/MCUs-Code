#include <Arduino.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/wifi/sta.h>
#include <eloquent_esp32cam/viz/image_collection.h>

#define WIFI_SSID "Mix_2s"
#define WIFI_PASS "enterolertcam"
#define HOSTNAME "esp32cam"

using eloq::camera;
using eloq::wifi;
using eloq::viz::collectionServer;


void setup() {
    delay(3000);
    Serial.begin(115200);
    Serial.println("___IMAGE COLLECTION SERVER___");

    // camera settings
    // replace with your own model!
    camera.pinout.aithinker();
    camera.brownout.disable();
    // Edge Impulse models work on square images
    // face resolution is 240x240
    camera.resolution.svga();
    // set xclk frequency to 20 MHz (default is 10 MHz)
    camera.xclk.fast();
    // set pixel format to JPEG (default is RGB565)
    // JPEG is required for Edge Impulse models
    // RGB565 is used for visualization purposes
    // and is not compatible with Edge Impulse models
    // if you want to use RGB565, use camera.pixformat.rgb565();
    camera.pixformat.jpeg();
    // set quality to 20 (default is 10)
    camera.quality.high();

    // init camera
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

    // connect to WiFi
    while (!wifi.connect(WIFI_SSID, WIFI_PASS).isOk())
      Serial.println(wifi.exception.toString());

    // init face detection http server
    while (!collectionServer.begin().isOk())
        Serial.println(collectionServer.exception.toString());

    Serial.println("Camera OK");
    Serial.println("WiFi OK");
    Serial.println("Image Collection Server OK");
    Serial.println(collectionServer.address());
}


void loop() {
    // server runs in a separate thread, no need to do anything here
}