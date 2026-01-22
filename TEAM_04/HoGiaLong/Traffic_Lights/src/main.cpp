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
  // 1. Đèn XANH sáng (các đèn khác tắt)
  digitalWrite(greenPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, LOW);
  delay(5000); // Chờ 5 giây (5000 mili giây)

  // 2. Đèn VÀNG sáng (các đèn khác tắt)
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(redPin, LOW);
  delay(2000); // Chờ 2 giây

  // 3. Đèn ĐỎ sáng (các đèn khác tắt)
  digitalWrite(greenPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(redPin, HIGH);
  delay(5000); // Chờ 5 giây

  // Sau khi hết 5 giây đèn đỏ, vòng lặp loop() sẽ quay lại từ đầu (Đèn xanh sáng)
}