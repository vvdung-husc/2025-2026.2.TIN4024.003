#include <Arduino.h>

uint8_t LED_RED = 17;

// Non-blocking timer: returns true every "millisecond" ms
bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

unsigned long ulEDT1 = 0;   // timer for LED blinking
bool ledState = false;

void setup() {
  Serial.begin(115200);               // để xem log trong Serial Monitor
  Serial.println("Welcome IoT");

  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);
}

void loop() {
  // Mỗi 500ms đổi trạng thái LED một lần (nháy)
  if (IsReady(ulEDT1, 500)) {
    ledState = !ledState;
    digitalWrite(LED_RED, ledState ? HIGH : LOW);

    Serial.printf("(LED_RED) => %s\n", ledState ? "HIGH" : "LOW");
  }

  // Ở đây vẫn có thể làm việc khác mà không bị delay chặn
  // ví dụ: đọc cảm biến, xử lý nút nhấn, wifi, mqtt, ...
}
