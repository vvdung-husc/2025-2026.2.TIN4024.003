#include <Arduino.h>

// Định nghĩa các chân nối với đèn LED (khớp với diagram.json)
#define LED_RED    23  // Đèn Đỏ
#define LED_YELLOW 22  // Đèn Vàng
#define LED_GREEN  21  // Đèn Xanh

void setup() {
  // Bật Serial để in thông báo ra màn hình (nếu cần xem)
  Serial.begin(115200);

  // Cấu hình các chân này là Output (để điều khiển điện ra)
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
}

void loop() {
  // --- PHA 1: ĐÈN ĐỎ SÁNG ---
  Serial.println("Den Do dang sang...");
  digitalWrite(LED_RED, HIGH);    // Bật Đỏ
  digitalWrite(LED_YELLOW, LOW);  // Tắt Vàng
  digitalWrite(LED_GREEN, LOW);   // Tắt Xanh
  delay(1000); // Đợi 1 giây (1000ms)

  // --- PHA 2: ĐÈN VÀNG SÁNG ---
  Serial.println("Den Vang dang sang...");
  digitalWrite(LED_RED, LOW);     // Tắt Đỏ
  digitalWrite(LED_YELLOW, HIGH); // Bật Vàng
  digitalWrite(LED_GREEN, LOW);   // Tắt Xanh
  delay(1000); // Đợi 1 giây

  // --- PHA 3: ĐÈN XANH SÁNG ---
  Serial.println("Den Xanh dang sang...");
  digitalWrite(LED_RED, LOW);     // Tắt Đỏ
  digitalWrite(LED_YELLOW, LOW);  // Tắt Vàng
  digitalWrite(LED_GREEN, HIGH);  // Bật Xanh
  delay(1000); // Đợi 1 giây
}