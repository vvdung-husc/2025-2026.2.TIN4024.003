#include "blynk_service.h"
#include <BlynkSimpleEsp8266.h>
#include <BlynkApiArduino.h>

#include "secrets.h"

static bool ledState = false;
static BlynkTimer blynkTimer;
static unsigned long lastBlynkConnectAttempt = 0;
static const unsigned long BLYNK_RECONNECT_INTERVAL = 5000; // Thử kết nối lại mỗi 5s

// Cú pháp chuẩn của Blynk để nhận tín hiệu từ nút bấm V1 trên app
BLYNK_WRITE(V1) {
  int v = param.asInt();
  ledState = (v != 0);
  digitalWrite(D5, ledState ? HIGH : LOW);
}

void blynkInit() {
  // Non-blocking configuration only
  Serial.print("Blynk config: ");
  Blynk.config(auth);
  Serial.println("done");
  lastBlynkConnectAttempt = millis();
  blynkTimer.setInterval(2000L, [](){ /* placeholder */ });
}

void blynkTick() {
  // Thử connect nếu chưa connect và đủ interval (non-blocking)
  if (!Blynk.connected() && (millis() - lastBlynkConnectAttempt >= BLYNK_RECONNECT_INTERVAL)) {
    Serial.print("Attempting Blynk connect...");
    if (Blynk.connect(5000)) {  // Timeout 5s
      Serial.println(" OK");
    } else {
      Serial.println(" failed");
    }
    lastBlynkConnectAttempt = millis();
  }
  
  // Chỉ run nếu connected để tránh block
  if (Blynk.connected()) {
    Blynk.run();
    blynkTimer.run();
  }
  yield(); // Feed watchdog
}

void blynkSendSensor(float temp, float hum, int gas) {
  Blynk.virtualWrite(V0, (millis() / 1000));
  if (!isnan(temp)) Blynk.virtualWrite(V2, temp);
  if (!isnan(hum)) Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gas);
  Blynk.virtualWrite(V5, "Team 05 - ESP8266_BLYNK_TELEGRAM");
  Blynk.virtualWrite(V1, ledState ? 1 : 0);
}

void blynkSetLed(bool on) {
  ledState = on;
  digitalWrite(D5, ledState ? HIGH : LOW);
  Blynk.virtualWrite(V1, ledState ? 1 : 0);
}

bool blynkGetLed() {
  return ledState;
}