#include <Arduino.h>

const int redPin = 23;
const int yellowPin = 22;
const int greenPin = 21;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop() {
  // --- 1. Đèn XANH nhấp nháy (5 giây) ---
  // Đảm bảo 2 đèn kia tắt trước khi bắt đầu
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, LOW);

  // Nhấp nháy 5 lần (mỗi lần 1 giây = 500ms sáng + 500ms tắt)
  for (int i = 0; i < 5; i++) {
    digitalWrite(greenPin, HIGH); // Bật
    delay(500);
    digitalWrite(greenPin, LOW);  // Tắt
    delay(500);
  }

  // --- 2. Đèn VÀNG nhấp nháy (2 giây) ---
  // Đảm bảo đèn xanh đã tắt (vòng lặp trên đã tắt nó rồi)
  
  // Nhấp nháy 4 lần nhanh hơn (mỗi lần 0.5 giây = 250ms sáng + 250ms tắt)
  for (int i = 0; i < 4; i++) {
    digitalWrite(yellowPin, HIGH); // Bật
    delay(250);
    digitalWrite(yellowPin, LOW);  // Tắt
    delay(250);
  }

  // --- 3. Đèn ĐỎ nhấp nháy (5 giây) ---
  // Tương tự đèn xanh
  for (int i = 0; i < 5; i++) {
    digitalWrite(redPin, HIGH); // Bật
    delay(500);
    digitalWrite(redPin, LOW);  // Tắt
    delay(500);
  }
  
  // Hết vòng lặp, quay lại đèn Xanh
}