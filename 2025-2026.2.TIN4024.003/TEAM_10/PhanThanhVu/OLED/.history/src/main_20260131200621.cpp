#include <Arduino.h>

// ====== PIN theo diagram của bạn ======
const int LDR_PIN   = 34;  // ldr1:AO -> esp:34

const int LED_RED   = 18;  // led1 (red)  -> esp:18
const int LED_YELLOW= 5;   // led3 (yellow)-> esp:5
const int LED_GREEN = 17;  // led2 (green)-> esp:17

// ====== NGƯỠNG ÁNH SÁNG ======
const int THRESHOLD = 1500;

// ====== HÀM NHÁY LED TRONG durationMs ======
// intervalMs: chu kỳ nháy (VD 500ms = nháy 2 lần/giây)
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
  int lightValue = analogRead(LDR_PIN); // ESP32 ADC: 0..4095 (Wokwi)
  Serial.print("Light = ");
  Serial.println(lightValue);

  if (lightValue < THRESHOLD) {
    // Dưới 1500: 3 đèn nhấp nháy lần lượt
    allOff();
    blinkFor(LED_GREEN, 10000, 500);   // xanh 10s
    blinkFor(LED_YELLOW, 5000, 500);   // vàng 5s
    blinkFor(LED_RED, 5000, 500);      // đỏ 5s (đổi nếu bạn muốn)
  } else {
    // Trên hoặc bằng 1500: chỉ vàng nháy, xanh & đỏ tắt
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);

    digitalWrite(LED_YELLOW, HIGH);
    delay(500);
    digitalWrite(LED_YELLOW, LOW);
    delay(500);
  }
}
