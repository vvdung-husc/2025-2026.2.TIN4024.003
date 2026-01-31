#include <Arduino.h>

// ========== KHAI BÁO CHÂN ==========
const uint8_t LED_RED    = 17;
const uint8_t LED_YELLOW = 16;
const uint8_t LED_GREEN  = 4;

const uint8_t POT_PIN = 34;   // Biến trở (ADC)

// ========== THỜI GIAN ==========
uint32_t RED_DURATION   = 5000;
uint32_t GREEN_DURATION = 5000;
const uint32_t YELLOW_DURATION = 2000;

// ========== TRẠNG THÁI FSM ==========
enum TrafficLightState {
  RED_LIGHT,
  GREEN_LIGHT,
  YELLOW_LIGHT
};

TrafficLightState currentState = RED_LIGHT;
unsigned long ulTimer = 0;

// ========== HÀM TIMER KHÔNG CHẶN ==========
bool isReady(unsigned long &timer, uint32_t interval) {
  if (millis() - timer >= interval) {
    timer = millis();
    return true;
  }
  return false;
}

// ========== HÀM ĐIỀU KHIỂN LED ==========
void setTrafficLight(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_GREEN, green);
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(POT_PIN, INPUT);

  setTrafficLight(false, false, false);

  Serial.println("=================================");
  Serial.println(" DEN GIAO THONG + BIEN TRO (ESP32)");
  Serial.println(" Bien tro: 2s -> 10s");
  Serial.println("=================================");

  currentState = RED_LIGHT;
  setTrafficLight(true, false, false);
  Serial.println("🔴 DEN DO");
}

// ========== LOOP ==========
void loop() {

  // ---- Đọc biến trở ----
  int potValue = analogRead(POT_PIN);          // 0 - 4095
  uint32_t dynamicTime = map(potValue, 0, 4095, 2000, 10000);

  RED_DURATION   = dynamicTime;
  GREEN_DURATION = dynamicTime;

  // Debug
  Serial.print("Bien tro = ");
  Serial.print(potValue);
  Serial.print(" | Thoi gian = ");
  Serial.print(dynamicTime);
  Serial.println(" ms");

  // ---- FSM ----
  switch (currentState) {

    case RED_LIGHT:
      if (isReady(ulTimer, RED_DURATION)) {
        currentState = GREEN_LIGHT;
        setTrafficLight(false, false, true);
        Serial.println("🟢 DEN XANH");
      }
      break;

    case GREEN_LIGHT:
      if (isReady(ulTimer, GREEN_DURATION)) {
        currentState = YELLOW_LIGHT;
        setTrafficLight(false, true, false);
        Serial.println("🟡 DEN VANG");
      }
      break;

    case YELLOW_LIGHT:
      if (isReady(ulTimer, YELLOW_DURATION)) {
        currentState = RED_LIGHT;
        setTrafficLight(true, false, false);
        Serial.println("🔴 DEN DO");
      }
      break;
  }
}
