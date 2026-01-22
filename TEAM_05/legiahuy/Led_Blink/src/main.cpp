#include <Arduino.h>

const int RED = 2;
const int YELLOW = 4;
const int GREEN = 5;

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // Đèn Đỏ sáng
  Serial.println("DUNG LAI - DEN DO");
  digitalWrite(RED, HIGH);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  delay(5000);

  // Đèn Xanh sáng
  Serial.println("DI TIEP - DEN XANH");
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, HIGH);
  delay(5000);

  // Đèn Vàng sáng
  Serial.println("CHUAN BI DUNG - DEN VANG");
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, HIGH);
  digitalWrite(GREEN, LOW);
  delay(2000);
}