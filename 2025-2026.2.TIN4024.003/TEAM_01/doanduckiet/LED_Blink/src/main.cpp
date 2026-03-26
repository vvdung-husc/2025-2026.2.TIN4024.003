#include <Arduino.h>

// Định nghĩa chân GPIO dựa trên connections trong diagram.json
const int RED_LED    = 25; // r1:2 nối với esp:25
const int YELLOW_LED = 33; // r2:2 nối với esp:33
const int GREEN_LED  = 32; // r3:2 nối với esp:32

void setup() {
  // Thiết lập các chân LED làm đầu ra
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Khởi tạo Serial để theo dõi trạng thái trên terminal
  Serial.begin(115200);
  Serial.println("--- Traffic Light Simulation Started ---");
}

void loop() {
  // 1. ĐÈN ĐỎ SÁNG (5 Giây)
  Serial.println("LED [RED   ] ON => 5 Seconds");
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  delay(5000);

  // 2. ĐÈN VÀNG SÁNG (2 Giây)
  Serial.println("LED [YELLOW] ON => 2 Seconds");
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  delay(2000);

  // 3. ĐÈN XANH SÁNG (7 Giây)
  Serial.println("LED [GREEN ] ON => 7 Seconds");
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  delay(7000);
}