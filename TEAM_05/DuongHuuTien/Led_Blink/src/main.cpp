#include <Arduino.h>

const uint8_t LED_RED = 17;
const uint8_t LED_YELLOW = 16;
const uint8_t LED_GREEN = 4;

enum TrafficState { RED, GREEN, YELLOW };
TrafficState currentState = RED;

unsigned long ulTimer = 0;       // Hẹn giờ chuyển trạng thái (Đỏ -> Xanh...)
unsigned long ulBlinkTimer = 0;  // Hẹn giờ nhấp nháy
bool ledStatus = HIGH;           // Trạng thái bật/tắt hiện tại của đèn nhấp nháy

// Hàm không gây nghẽn
bool IsReady(unsigned long &timer, uint32_t interval) {
  if (millis() - timer < interval) return false;
  timer = millis();
  return true;
}

// Hàm hỗ trợ nhấp nháy đèn bất kỳ
void BlinkLED(uint8_t pin, uint32_t speed) {
  if (IsReady(ulBlinkTimer, speed)) {
    ledStatus = !ledStatus;
    digitalWrite(pin, ledStatus);
  }
}

// Tắt tất cả các đèn trước khi chuyển trạng thái
void AllOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  ledStatus = HIGH; // Reset trạng thái để đèn sau bắt đầu bằng mức HIGH
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  digitalWrite(LED_RED, HIGH); // Bắt đầu với đèn Đỏ
}

void loop() {
  switch (currentState) {
    case RED:
      // Nhấp nháy đèn Đỏ mỗi 500ms
      BlinkLED(LED_RED, 500);

      // Sau 5 giây thì chuyển sang Xanh
      if (IsReady(ulTimer, 5000)) {
        AllOff();
        digitalWrite(LED_GREEN, HIGH);
        currentState = GREEN;
        printf("[SWITCH] => GREEN\n");
      }
      break;

    case GREEN:
      // Nhấp nháy đèn Xanh mỗi 300ms (nhanh hơn chẳng hạn)
      BlinkLED(LED_GREEN, 300);

      if (IsReady(ulTimer, 4000)) {
        AllOff();
        digitalWrite(LED_YELLOW, HIGH);
        currentState = YELLOW;
        printf("[SWITCH] => YELLOW\n");
      }
      break;

    case YELLOW:
      // Nhấp nháy đèn Vàng mỗi 200ms
      BlinkLED(LED_YELLOW, 200);

      if (IsReady(ulTimer, 2000)) {
        AllOff();
        digitalWrite(LED_RED, HIGH);
        currentState = RED;
        printf("[SWITCH] => RED\n");
      }
      break;
  }
}