#include <Arduino.h>
#include "DHT.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <HTTPClient.h>

// --- Cấu hình ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";
// Lưu ý: Dùng http thay vì https trong giả lập thường ổn định hơn
String serverPath = "http://api.thingspeak.com/update?api_key=AXUI3SE8C892K67Q";

#define DHTPIN 4
#define DHTTYPE DHT22
#define SD_CS 5
#define LED_PIN 2  // Chân LED theo diagram.json của bạn

DHT dht(DHTPIN, DHTTYPE);

// --- Khai báo Prototypes ---
void connectWiFi();
void saveToSD(float temp, float hum);
void sendToThingSpeak(float temp, float hum);
void blinkLED(int times, int duration);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n--- HE THONG KHOI HANH ---"));

  // Cấu hình LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card: That bai!"));
  } else {
    Serial.println(F("SD Card: San sang."));
    File file = SD.open("/data_log.csv", FILE_APPEND);
    if (file && file.size() == 0) {
      file.println("Time(ms),Temp(C),Hum(%)");
      file.close();
    }
  }

  connectWiFi();
  
  // Nháy đèn 3 lần báo hiệu hệ thống đã sẵn sàng
  blinkLED(3, 100);
}

void loop() {
  // ThingSpeak cần giãn cách 15s
  delay(15000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Loi doc DHT22!"));
    return;
  }

  Serial.printf("Data: T=%.2f, H=%.2f\n", t, h);

  // 1. Ghi vào SD và nháy đèn
  saveToSD(t, h);

  // 2. Gửi lên web và nháy đèn
  sendToThingSpeak(t, h);
}

// --- Định nghĩa hàm ---

void blinkLED(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(duration);
    digitalWrite(LED_PIN, LOW);
    delay(duration);
  }
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("WiFi: Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi: OK!");
}

void saveToSD(float temp, float hum) {
  Serial.println("-> Dang ghi SD...");
  File dataFile = SD.open("/data_log.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%lu,%.2f,%.2f\n", millis(), temp, hum);
    dataFile.close();
    Serial.println("-> SD: Ghi thanh cong.");
    
    // Nháy đèn 1 lần khi ghi xong SD
    blinkLED(1, 200);
  } else {
    Serial.println("-> SD: Loi mo file!");
  }
}

void sendToThingSpeak(float temp, float hum) {
  Serial.println("-> Dang gui Web...");
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  HTTPClient http;
  char query[64];
  sprintf(query, "&field1=%.2f&field2=%.2f", temp, hum);
  String fullUrl = serverPath + String(query);

  http.begin(fullUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.printf("-> Web Code: %d\n", httpResponseCode);
    // Nháy đèn 2 lần khi gửi thành công lên Web
    if (httpResponseCode == 200) {
        blinkLED(2, 100);
    }
  } else {
    Serial.printf("-> Web Error: %d\n", httpResponseCode);
  }
  http.end();
}