#include <Arduino.h>

// Định nghĩa các chân cắm dựa trên sơ đồ của bạn
#define LED_GREEN  19
#define LED_YELLOW 18
#define LED_RED    5

void setup() {
  Serial.begin(115200);
  Serial.println("He thong den giao thong dang khoi dong...");

  // Cấu hình tất cả các chân là OUTPUT
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  // 1. ĐÈN XANH SÁNG (Ví dụ: 5 giây)
  Serial.println("DEN XANH");
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  delay(5000); 

  // 2. ĐÈN VÀNG SÁNG (Ví dụ: 2 giây)
  Serial.println("DEN VANG");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_RED, LOW);
  delay(2000);

  // 3. ĐÈN ĐỎ SÁNG (Ví dụ: 5 giây)
  Serial.println("DEN DO");
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  delay(5000);
}