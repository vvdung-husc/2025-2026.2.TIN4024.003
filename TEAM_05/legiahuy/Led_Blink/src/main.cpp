#include <Arduino.h>

// Định nghĩa chân kết nối
const int RED_PIN = 2;
const int YELLOW_PIN = 4;
const int GREEN_PIN = 5;

void setup() {
  // Cấu hình chân Output
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  
  // Khởi tạo Serial để theo dõi trên màn hình
  Serial.begin(115200);
  Serial.println("He thong den giao thong bat dau...");
}

// Hàm giúp đèn nhấp nháy trong một khoảng thời gian nhất định
void blinkLight(int pin, String colorName, int seconds) {
  Serial.print("Dang nhap nhay den: ");
  Serial.println(colorName);
  
  for (int i = 0; i < seconds; i++) {
    digitalWrite(pin, HIGH); 
    delay(500);
    digitalWrite(pin, LOW); 
    delay(500);
  }
}

void loop() {
  // 1. Đèn Đỏ nhấp nháy trong 5 giây
  blinkLight(RED_PIN, "DO", 5);

  // 2. Đèn Xanh nhấp nháy trong 5 giây
  blinkLight(GREEN_PIN, "XANH", 5);

  // 3. Đèn Vàng nhấp nháy trong 2 giây
  blinkLight(YELLOW_PIN, "VANG", 2);
}