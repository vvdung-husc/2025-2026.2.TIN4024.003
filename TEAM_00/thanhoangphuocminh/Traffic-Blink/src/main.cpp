#include <Arduino.h>

// Định nghĩa chân kết nối (Khớp với sơ đồ trong ảnh)
#define RED_PIN    25   // Đèn Đỏ
#define GREEN_PIN  32   // Đèn Xanh
#define YELLOW_PIN 33   // Đèn Vàng

void setup() {
  // Cấu hình các chân làm đầu ra (Output)
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
}

void loop() {
  // 1. ĐÈN ĐỎ SÁNG (10 giây)
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  delay(10000); // 10s

  // 2. ĐÈN XANH SÁNG (7 giây)
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(YELLOW_PIN, LOW);
  delay(7000);  // 7s

  // 3. ĐÈN VÀNG SÁNG (3 giây)
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);
  delay(3000);  // 3s
}