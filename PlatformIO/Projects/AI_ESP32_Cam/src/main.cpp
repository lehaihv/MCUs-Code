#include <Arduino.h>
#include "esp_camera.h"
#include "SPI.h"
#include "driver/rtc_io.h"
#include <ESP_Mail_Client.h>
#include <FS.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <time.h>
#include <SD_MMC.h>

// variables
char send_time[30];

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Mix_2s";
const char* password = "enterolertcam";

// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
#define emailSenderAccount    "hai.le.uf.2024@gmail.com"
#define emailSenderPassword   "bclzgrxzxcfgoeea"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "Enterolert-Cam Photo Captured"
#define emailRecipient        "haiirfvn1124@gmail.com"

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

SMTPSession smtp;

void smtpCallback(SMTP_Status status);
void capturePhotoSaveLittleFS(void);
void sendPhoto(void);
void sync_time(void);

#define FILE_PHOTO "photo.jpg"
#define FILE_PHOTO_PATH "/photo.jpg"

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.println();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Sync time
  sync_time();

  // Init filesystem for ESP Mail Client
  ESP_MAIL_DEFAULT_FLASH_FS.begin();
  // LittleFS.begin(true); // Initialize LittleFS with format on fail
  

  // Initialize SD card (1-bit mode for reliability)
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failed");
  } else {
    Serial.println("SD card initialized.");
  }

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

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // set camera parameters
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, -2);
  s->set_contrast(s, 2);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 3);
  s->set_exposure_ctrl(s, 0);
  s->set_aec2(s, 0);
  s->set_ae_level(s, 0);
  s->set_aec_value(s, 600);
  s->set_gain_ctrl(s, 0);
  s->set_agc_gain(s, 5);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_hmirror(s, 0);
  s->set_vflip(s, 0);
  s->set_dcw(s, 1);
  s->set_colorbar(s, 0);

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  capturePhotoSaveLittleFS();
  sendPhoto();
  esp_sleep_enable_timer_wakeup(2*60*1000*1000);
  esp_deep_sleep_start();
}

void loop() {
  delay(60000);
}

// Capture Photo and Save it to ESP_MAIL_DEFAULT_FLASH_FS and SD card
void capturePhotoSaveLittleFS(void) {
  camera_fb_t* fb = NULL;
  for (int i = 0; i < 3; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  // Save to flash (for email)
  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  File file = ESP_MAIL_DEFAULT_FLASH_FS.open(FILE_PHOTO_PATH, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len);
    Serial.print("The picture has been saved in ");
    Serial.print(FILE_PHOTO_PATH);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  file.close();

  // Save to SD card with timestamped filename
  String sdFileName = "/photo_";
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%Y%m%d_%H%M%S", &timeinfo);
  sdFileName += timeString;
  sdFileName += ".jpg";

  // Define age threshold (7 days in seconds)
  const time_t ageThreshold = 7 * 24 * 60 * 60;
  now = time(nullptr);

  // Delete all .jpg files older than 1 week
  File root = SD_MMC.open("/");
  int deletedCount = 0;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    String name = entry.name();
    if (name.endsWith(".jpg")) {
      time_t t = entry.getLastWrite();
      if ((now - t) > ageThreshold) {
        Serial.print("Deleting old file: ");
        Serial.println(name);
        entry.close();
        SD_MMC.remove(name);
        deletedCount++;
        continue;
      }
    }
    entry.close();
  }
  root.close();
  if (deletedCount > 0) {
    Serial.print("Deleted ");
    Serial.print(deletedCount);
    Serial.println(" old .jpg files.");
  }

  // Recalculate free space after deletion
  uint64_t usedBytes = SD_MMC.usedBytes();
  uint64_t freeBytes = SD_MMC.totalBytes() - usedBytes;

  // If still not enough space, you can optionally delete more (e.g., oldest remaining files)
  while (freeBytes < fb->len) {
    File root = SD_MMC.open("/");
    File oldestFile;
    time_t oldestTime = now;
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      String name = entry.name();
      if (name.endsWith(".jpg")) {
        time_t t = entry.getLastWrite();
        if (t < oldestTime) {
          oldestTime = t;
          if (oldestFile) oldestFile.close();
          oldestFile = entry;
        } else {
          entry.close();
        }
      } else {
        entry.close();
      }
    }
    if (oldestFile) {
      Serial.print("Deleting oldest file: ");
      Serial.println(oldestFile.name());
      String toDelete = oldestFile.name();
      oldestFile.close();
      SD_MMC.remove(toDelete);
      usedBytes = SD_MMC.usedBytes();
      freeBytes = SD_MMC.totalBytes() - usedBytes;
    } else {
      Serial.println("No .jpg files to delete, cannot free space!");
      break;
    }
  }

  File sdfile = SD_MMC.open(sdFileName, FILE_WRITE);
  if (!sdfile) {
    Serial.println("Failed to open file on SD card");
  } else {
    sdfile.write(fb->buf, fb->len);
    Serial.print("The picture has been saved to SD card as ");
    Serial.println(sdFileName);
  }
  sdfile.close();

  esp_camera_fb_return(fb);
}

void sendPhoto(void) {
  smtp.debug(1);
  smtp.callback(smtpCallback);

  Session_Config config;
  config.server.host_name = smtpServer;
  config.server.port = smtpServerPort;
  config.login.email = emailSenderAccount;
  config.login.password = emailSenderPassword;
  config.login.user_domain = "";

  SMTP_Message message;
  message.enable.chunking = true;
  message.sender.name = "ESP32-CAM";
  message.sender.email = emailSenderAccount;
  message.subject = emailSubject;
  message.addRecipient("Lab", emailRecipient);

  String htmlMsg = "<h2>Photo captured with ESP32-CAM and attached in this email.</h2>";
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_qp;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  SMTP_Attachment att;
  time_t current_time;
  struct tm* timeinfo;
  time(&current_time);
  timeinfo = localtime(&current_time);
  strftime(send_time, sizeof(send_time), "%Y%m%d_%H%M%S", timeinfo);
  String file_name = "Photo_";
  file_name += send_time;
  file_name += ".jpg";
  att.descr.filename = file_name;
  att.descr.mime = "image/jpeg";
  att.file.path = FILE_PHOTO_PATH;
  att.file.storage_type = esp_mail_file_storage_type_flash;
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

  message.addAttachment(att);

  if (!smtp.connect(&config))
    return;

  bool mail_send_status = false;
  int8_t resend_times = 0;
  while (!mail_send_status && resend_times < 2) {
    if (MailClient.sendMail(&smtp, &message, true)) {
      mail_send_status = true;
    } else {
      Serial.printf("Error sending Email: %s\n", smtp.errorReason().c_str());
      Serial.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
      resend_times++;
      Serial.printf("Resend the image %d times\n", resend_times);
      delay(5000);
    }
    smtp.closeSession();
  }
  if (!mail_send_status) {
    Serial.println("Fail to send after 2 attempts.");
    delay(2000); // Optional: allow time for serial message to be sent
    ESP.restart(); // Restart the ESP32
  }
}

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;
    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
    Serial.println(send_time);
    smtp.sendingResult.clear();
  }
}

void sync_time(void) {
  const char* ntpServer = "pool.ntp.org";
  configTime(-4*3600, 0, ntpServer); // GMT-4, no DST
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Time synchronized");
  time_t now = time(nullptr);
  Serial.print("Current time: ");
  Serial.println(ctime(&now));
}