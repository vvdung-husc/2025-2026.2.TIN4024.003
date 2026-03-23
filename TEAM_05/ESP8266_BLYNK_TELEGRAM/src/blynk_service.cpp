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
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
}

void blynkInit() {
  // Non-blocking configuration only
  Serial.print("Blynk config: ");
  Blynk.config(auth);
  Serial.println("done (NOT attempting to connect in init)");
  // Blynk.connect() is blocking - will only be called once async attempt has started
  lastBlynkConnectAttempt = millis();
  blynkTimer.setInterval(2000L, [](){ /* placeholder */ });
}

void blynkTick() {
  // CRITICAL: Do NOT call Blynk.connect() from loop - it blocks and triggers Soft WDT!
  // Only run Blynk if already connected, to avoid blockingoperations
  if (Blynk.connected()) {
    Blynk.run();
    blynkTimer.run();
  }
  yield(); // Feed watchdog timer
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
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
  Blynk.virtualWrite(V1, ledState ? 1 : 0);
}

bool blynkGetLed() {
  return ledState;
}