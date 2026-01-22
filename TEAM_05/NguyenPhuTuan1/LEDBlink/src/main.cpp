#include <Arduino.h>

// Pin số có đèn LED kết nối
#define LED_PIN 17

void setup() {
  // Khởi tạo chân LED là đầu ra
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Bật đèn LED
  digitalWrite(LED_PIN, HIGH);
  delay(500);  // Chờ 500ms
  
  // Tắt đèn LED
  digitalWrite(LED_PIN, LOW);
  delay(500);  // Chờ 500ms
}
