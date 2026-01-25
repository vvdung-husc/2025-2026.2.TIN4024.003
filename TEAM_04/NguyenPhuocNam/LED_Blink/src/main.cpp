#include <Arduino.h>

#define LED_PIN 23      // Chân đèn LED
#define LDR_PIN 34      // Chân Quang trở (LDR)

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
}

void loop() {
  // 1. Đọc giá trị ánh sáng (0: Tối om, 4095: Sáng chói)
  int lightValue = analogRead(LDR_PIN);
  
  // 2. Chuyển đổi giá trị đọc được thành thời gian delay hợp lý
  // Chia 5 để số nhỏ đi một chút cho đèn chớp nhanh hơn
  int delayTime = lightValue / 5;

  Serial.print("Do sang: ");
  Serial.print(lightValue);
  Serial.print(" -> Toc do chop: ");
  Serial.println(delayTime);

  // 3. Chớp tắt đèn
  digitalWrite(LED_PIN, HIGH);
  delay(delayTime); 
  
  digitalWrite(LED_PIN, LOW);
  delay(delayTime);
}