#include <Arduino.h>

// ====== PIN theo diagram ======
const int LDR_PIN   = 34;

const int LED_RED    = 18;  // led1
const int LED_YELLOW = 5;   // led3
const int LED_GREEN  = 17;  // led2

const int THRESHOLD = 1500;

// ====== HÀM NHÁY LED ======
void blinkFor(int pin, unsigned long durationMs, unsigned long intervalMs = 500) {
  unsigned long start = millis();
  bool state = false;

  while (millis() - start < durationMs) {
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
    delay(intervalMs);
  }
  digitalWrite(pin, LOW); // kết thúc thì tắt
}

void allOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  allOff();
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  Serial.println(lightValue);

  if (lightValue < THRESHOLD) {
    // Ánh sáng yếu
    allOff();
    blinkFor(LED_GREEN, 10000);   // xanh 10s
    blinkFor(LED_YELLOW, 5000);   // vàng 5s
    blinkFor(LED_RED, 10000);     // đỏ 10s
  } else {
    // Ánh sáng mạnh
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);

    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
  }
}
