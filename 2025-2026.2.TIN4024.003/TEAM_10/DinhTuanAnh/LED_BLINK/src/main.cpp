#include <Arduino.h>

#define LED_RED     18
#define LED_YELLOW  5
#define LED_GREEN   17
#define LDR_PIN     34

int lightThreshold = 2000;   // ngưỡng sáng/tối

void blinkLED(int ledPin, const char* name);
void nightMode();

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int rawValue = analogRead(LDR_PIN);

  // 🔥 ĐẢO GIÁ TRỊ (để thấp = sáng, cao = tối)
  int lightValue = 4095 - rawValue;

  Serial.print("Light Value (Adjusted): ");
  Serial.println(lightValue);

  // ☀️ GIÁ TRỊ THẤP = SÁNG
  if (lightValue < lightThreshold) {
    Serial.println("=== DAY MODE ===");
    blinkLED(LED_GREEN, "GREEN");
    blinkLED(LED_YELLOW, "YELLOW");
    blinkLED(LED_RED, "RED");
  }
  // 🌙 GIÁ TRỊ CAO = TỐI
  else {
    nightMode();
  }
}

// ====== NHẤP NHÁY 5 GIÂY ======
void blinkLED(int ledPin, const char* name) {
  Serial.print("Blinking LED: ");
  Serial.println(name);

  unsigned long start = millis();
  while (millis() - start < 5000) {
    digitalWrite(ledPin, HIGH);
    delay(300);
    digitalWrite(ledPin, LOW);
    delay(300);
  }
}

// ====== CHẾ ĐỘ TỐI ======
void nightMode() {
  Serial.println("=== NIGHT MODE (Yellow only) ===");

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  delay(500);
  digitalWrite(LED_YELLOW, LOW);
  delay(500);
}

