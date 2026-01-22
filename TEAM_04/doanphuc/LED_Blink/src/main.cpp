#include <Arduino.h>

// Định nghĩa chân số 5 theo như sơ đồ dây nối của bạn
#define LED_PIN 5 

void setup() {
  // Khởi tạo cổng Serial để xem log (thay cho printf trong Arduino)
  Serial.begin(115200);
  Serial.println("Welcome IoT");

  // Cấu hình chân LED_PIN là đầu ra
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Bật đèn LED
  Serial.println("[LED_RED]_HIGH");
  digitalWrite(LED_PIN, HIGH);
  delay(1000); // Đợi 1 giây (1000ms)

  // Tắt đèn LED
  Serial.println("[LED_RED]_LOW");
  digitalWrite(LED_PIN, LOW);
  delay(1000); // Đợi 1 giây
}