#include <Arduino.h>

const int redPin = 25;    
const int yellowPin = 33; 
const int greenPin = 32;  

void setup() {
 
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop() {
  // --- GIAI ĐOẠN 1: ĐÈN ĐỎ SÁNG (Dừng lại) ---
  digitalWrite(redPin, HIGH);    // Bật đèn Đỏ
  digitalWrite(yellowPin, LOW);  // Tắt đèn Vàng
  digitalWrite(greenPin, LOW);   // Tắt đèn Xanh
  delay(1000); // Chờ 5 giây (5000 mili giây)

  // --- GIAI ĐOẠN 2: ĐÈN XANH SÁNG (Được đi) ---
  digitalWrite(redPin, LOW);     // Tắt đèn Đỏ
  digitalWrite(yellowPin, LOW);  // Tắt đèn Vàng
  digitalWrite(greenPin, HIGH);  // Bật đèn Xanh
  delay(1000); // Chờ 4 giây

  // --- GIAI ĐOẠN 3: ĐÈN VÀNG SÁNG (Chuẩn bị dừng) ---
  digitalWrite(redPin, LOW);     // Tắt đèn Đỏ
  digitalWrite(yellowPin, HIGH); // Bật đèn Vàng
  digitalWrite(greenPin, LOW);   // Tắt đèn Xanh
  delay(1000); // Chờ 2 giây
  
  // Sau khi hết giai đoạn 3, vòng lặp loop() sẽ quay lại từ đầu (Đèn Đỏ sáng)
}