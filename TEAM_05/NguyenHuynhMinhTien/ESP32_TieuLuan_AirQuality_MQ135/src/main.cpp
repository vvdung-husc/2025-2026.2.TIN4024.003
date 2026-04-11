#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Blynk Auth Token
#define BLYNK_TEMPLATE_ID "TMPL6EPuDeoFw"
#define BLYNK_TEMPLATE_NAME "AirQualityMQ135"
#define BLYNK_AUTH_TOKEN "eH1zWFtc7jg8AWx1JJHUZLiLPpzGw0hd"

// Cảm biến MQ-135
#define MQ135_PIN A0
#define LED_PIN 17

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Virtual Pins
#define VPIN_CO2_VALUE V0      // Hiển thị nồng độ CO2
#define VPIN_CO2_GAUGE V1      // Gauge meter
#define VPIN_STATUS V2         // Trạng thái kết nối
#define VPIN_LED_CONTROL V3    // Điều khiển LED

float readMQ135() {
  int analogValue = analogRead(MQ135_PIN);
  float voltage = analogValue * (3.3 / 4095.0);
  float concentration = voltage * 100;
  return concentration;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  
  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void loop() {
  if (Blynk.connected()) {
    float concentration = readMQ135();
    
    // Gửi dữ liệu lên Blynk
    Blynk.virtualWrite(VPIN_CO2_VALUE, concentration);
    Blynk.virtualWrite(VPIN_CO2_GAUGE, concentration);
    Blynk.virtualWrite(VPIN_STATUS, "Connected");
    
    delay(1000);
  }
  
  Blynk.run();
}

// Nhận lệnh từ Blynk
BLYNK_WRITE(VPIN_LED_CONTROL) {
  int ledValue = param.asInt();
  digitalWrite(LED_PIN, ledValue);
}