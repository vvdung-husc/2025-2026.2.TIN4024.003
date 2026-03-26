#include "main.h"

#define PIN_LED_RED     18
#define PIN_LED_YELLOW  5
#define PIN_LED_GREEN    17


#define PIN_LDR 34

int DAY_ADC_THRESHOLD = 2000;

Trafic_Blink traficLight;
LDR ldrSensor;

void setup() {
  printf("Welcome IoT\n");

  ldrSensor.setup(PIN_LDR, false);

  traficLight.setupPin(PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_GREEN);
  traficLight.setupWaitTime(5, 3, 7);
}

void loop() {
  int analogValue = 0;
  float lux =ldrSensor.readLux(&analogValue);
  bool isDark = (analogValue > DAY_ADC_THRESHOLD);
  traficLight.blink(500, isDark);

}



