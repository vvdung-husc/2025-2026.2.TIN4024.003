#include "blynk_service.h"
#include <BlynkSimpleEsp8266.h>
#include <BlynkApiArduino.h>

#include "secrets.h"

static bool ledState = false;
static BlynkTimer blynkTimer;

// Cú pháp chuẩn của Blynk để nhận tín hiệu từ nút bấm V1 trên app
BLYNK_WRITE(V1) {
  int v = param.asInt();
  ledState = (v != 0);
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
}

void blynkInit() {
  // Set timeout cho Blynk.begin() để không hang
  Blynk.config(auth);
  Blynk.connect();
  // Đợi tối đa 5 giây để Blynk kết nối, nếu không thì tiếp tục
  unsigned long start = millis();
  while (!Blynk.connected() && millis() - start < 5000) {
    delay(100);
  }
  Serial.print("Blynk connected: ");
  Serial.println(Blynk.connected() ? "YES" : "NO");
  blynkTimer.setInterval(2000L, [](){ /* placeholder */ });
}

void blynkTick() {
  Blynk.run();
  blynkTimer.run();
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