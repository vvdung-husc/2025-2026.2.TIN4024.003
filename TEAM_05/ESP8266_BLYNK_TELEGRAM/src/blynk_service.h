#ifndef BLYNK_SERVICE_H
#define BLYNK_SERVICE_H

#include <Arduino.h>

void blynkInit();
void blynkTick();
void blynkSendSensor(float temp, float hum, int gas);
void blynkSetLed(bool on);
bool blynkGetLed();

#endif // BLYNK_SERVICE_H
