#include "blynk_service.h"
#include <BlynkSimpleEsp8266.h>
#include <BlynkApiArduino.h>

#include "secrets.h"

static bool ledState = false;
static BlynkTimer blynkTimer;

void onVirtualLedWrite(const BlynkParam &param) {
  int v = param[0].asInt();
  ledState = (v != 0);
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
}

void blynkInit() {
  // Blynk tự handle kết nối
  Blynk.begin(auth, WIFI_SSID, WIFI_PASS);
  Blynk.attachVirtual(V1, onVirtualLedWrite);
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
