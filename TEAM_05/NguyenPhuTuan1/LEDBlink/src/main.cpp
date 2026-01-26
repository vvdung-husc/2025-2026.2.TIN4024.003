#include <Arduino.h>

// Chân kết nối LED (chỉnh theo phần cứng của bạn nếu cần)
#define GREEN_PIN 17
#define YELLOW_PIN 16
#define RED_PIN 15

// Thời lượng (tính theo giây) cho từng màu
const unsigned int GREEN_DURATION = 5;
const unsigned int YELLOW_DURATION = 7;
const unsigned int RED_DURATION = 10;

void setup() {
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);

  // Đảm bảo tất cả LED tắt lúc bắt đầu
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
}

// Nháy một lần (1Hz): ON 500ms, OFF 500ms
void blinkOnce(int pin) {
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
  delay(500);
  // Đảm bảo LED được tắt hoàn toàn
  digitalWrite(pin, LOW);
}

void loop() {
  // Màu xanh trong GREEN_DURATION giây
  for (unsigned int i = 0; i < GREEN_DURATION; ++i) {
    blinkOnce(GREEN_PIN);
  }

  // Màu vàng trong YELLOW_DURATION giây
  for (unsigned int i = 0; i < YELLOW_DURATION; ++i) {
    blinkOnce(YELLOW_PIN);
  }

  // Màu đỏ trong RED_DURATION giây
  for (unsigned int i = 0; i < RED_DURATION; ++i) {
    blinkOnce(RED_PIN);
  }

  // Đảm bảo tất cả LED tắt hoàn toàn
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
}
