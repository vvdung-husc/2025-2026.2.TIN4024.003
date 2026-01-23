#include <Arduino.h>

#define LED1 17
#define LED2 18
#define LED3 19

unsigned long ulTimer = 0;
uint8_t currentLED = 0;
const uint32_t INTERVAL = 1000; // 1 giây

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Tắt tất cả LED ban đầu
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  printf("Start LED sequence (Non-blocking)\n");
}

void loop() {
  if (millis() - ulTimer >= INTERVAL) {
    ulTimer = millis();

    // Tắt tất cả LED
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);

    // Bật LED theo thứ tự
    if (currentLED == 0) {
      digitalWrite(LED1, HIGH);
      printf("LED1 ON\n");
    }
    else if (currentLED == 1) {
      digitalWrite(LED2, HIGH);
      printf("LED2 ON\n");
    }
    else if (currentLED == 2) {
      digitalWrite(LED3, HIGH);
      printf("LED3 ON\n");
    }

    // Chuyển sang LED tiếp theo
    currentLED++;
    if (currentLED > 2) currentLED = 0;
  }

  // 👇 chỗ này vẫn có thể làm việc khác
  // đọc nút nhấn
  // đọc cảm biến
  // xử lý WiFi
}
