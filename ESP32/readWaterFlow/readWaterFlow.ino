#include "proj_configs.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

#define CHUNK_LENGTH 1024
#define COUNTER_RESET 0

#define FLASH 4
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22



boolean connected = false;
WiFiUDP udp;

RTC_DATA_ATTR int bootCount = 0;

void setup() {

  Serial.begin(115200);
  
  pinMode(FLASH, OUTPUT);
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA; 
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.print("Boot count: ");
  Serial.println(bootCount);
  initSD();
  //Connect to the WiFi network
  connectToWiFi(ssid, password);
}

void initSD(){
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
   
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  if (COUNTER_RESET == 1) {
    File file = SD_MMC.open("/imgCounter.txt", FILE_WRITE);
       
    if (!file) {
      Serial.println("Opening file to write failed");
      return;
    }
    int init = 0;
    if (file.write(init)) {
      Serial.println("File write success");
    } else {
      Serial.println("File write failed");
    }
     
    file.close();
  }
}

int counter = 0;

void loop() {
  //only send data when connected
  if (connected) {
    counter = 0;
    camera_fb_t* fb = NULL;
    digitalWrite(FLASH, HIGH);
    delay(1000);
    esp_err_t res = ESP_OK;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      esp_camera_fb_return(fb);
      return;
    }

    if (fb->format != PIXFORMAT_JPEG) {
      Serial.println("PIXFORMAT_JPEG not implemented");
      esp_camera_fb_return(fb);
      return;
    }
    delay(1000);
    digitalWrite(FLASH, LOW);
    saveToSD(fb);
    sendPacketData((const char*)fb->buf, fb->len, CHUNK_LENGTH);
    esp_camera_fb_return(fb);
   
    goToSleep(3600);
  }
  
  delay(5000);
  counter++;
  if (counter >= 5){
    goToSleep(60);  
  }
}

void saveToSD(camera_fb_t* fb){
  File file = SD_MMC.open("/imgCounter.txt", FILE_READ);

  if (!file) {
    Serial.println("Opening file to read failed");
    return;
  }
  int imgCounter;
  
  while (file.available()) {
    imgCounter = file.read();
  }
  Serial.print("Pictures taken: ");
  Serial.println(imgCounter);
  file.close();
  File file2 = SD_MMC.open("/imgCounter.txt", FILE_WRITE);
     
  if (!file2) {
    Serial.println("Opening file to write failed");
    return;
  }
  imgCounter += 1;
  if (file2.write(imgCounter)) {
    Serial.println("File write success");
  } else {
    Serial.println("File write failed");
  }
   
  file2.close();
}

void goToSleep(uint64_t sec){
    disableWiFi();
    ++bootCount;
    pinMode(FLASH, OUTPUT);
    digitalWrite(FLASH, LOW);
    gpio_hold_en(GPIO_NUM_4);
    gpio_deep_sleep_hold_en();
    Serial.println("Going to Deepsleep");
    Serial.println(sec);
    esp_sleep_enable_timer_wakeup(sec * 1000 * 1000);
    esp_deep_sleep_start();
}

void disableWiFi(){
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

void connectToWiFi(const char* ssid, const char* pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
  }
}

void sendPacketData(const char* buf, uint64_t len, uint16_t chunkLength) {
  uint8_t buffer[chunkLength];
  size_t blen = sizeof(buffer);
  size_t rest = len % blen;
  Serial.println("Sending UDP packet");
  byte byteArray[6] = "start";
  udp.beginPacket(udpAddress,udpPort);
  udp.write(byteArray,6);
  udp.endPacket();
  delay(500);
  for (uint8_t i = 0; i < len / blen; ++i) {
    memcpy(buffer, buf + (i * blen), blen);
    udp.beginPacket(udpAddress, udpPort);
    udp.write(buffer, chunkLength);
    udp.endPacket();
    delay(10);
  }

  if (rest) {
    memcpy(buffer, buf + (len - rest), rest);
    udp.beginPacket(udpAddress, udpPort);
    udp.write(buffer, rest);
    udp.endPacket();
  }

  delay(500);
  
  byte byteArray2[6] = "stop";
  udp.beginPacket(udpAddress,udpPort);
  udp.write(byteArray2,6);
  udp.endPacket();
  delay(100);
}
