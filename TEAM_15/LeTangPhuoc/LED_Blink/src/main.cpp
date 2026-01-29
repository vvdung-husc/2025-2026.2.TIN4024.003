#include <Arduino.h>

/* ====== KHAI BÁO CHÂN LED ====== */
#define PIN_LED_RED     18
#define PIN_LED_YELLOW  5
#define PIN_LED_GREEN   17

/* ====== BIẾN THỜI GIAN ====== */
unsigned long timerTraffic = 0;

/* ====== TRẠNG THÁI ĐÈN ====== */
enum TrafficState {
  RED,
  YELLOW,
  GREEN
};

TrafficState currentState = RED;

/* ====== HÀM KIỂM TRA THỜI GIAN (NON-BLOCKING) ====== */
bool IsReady(unsigned long &timer, uint32_t interval) {
  if (millis() - timer < interval) return false;
  timer = millis();
  return true;
}

/* ====== SETUP ====== */
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Traffic Light Started");

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}

/* ====== LOOP ====== */
void loop() {
  switch (currentState) {

    case RED:
      if (IsReady(timerTraffic, 5000)) {   // Đỏ 5 giây
        Serial.println("RED -> GREEN");
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_GREEN, HIGH);
        currentState = GREEN;
      }
      break;

    case GREEN:
      if (IsReady(timerTraffic, 4000)) {   // Xanh 4 giây
        Serial.println("GREEN -> YELLOW");
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_YELLOW, HIGH);
        currentState = YELLOW;
      }
      break;

    case YELLOW:
      if (IsReady(timerTraffic, 2000)) {   // Vàng 2 giây
        Serial.println("YELLOW -> RED");
        digitalWrite(PIN_LED_YELLOW, LOW);
        digitalWrite(PIN_LED_RED, HIGH);
        currentState = RED;
      }
      break;
  }
}
