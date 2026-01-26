#include <Arduino.h>

#define LED1 17
#define LED2 18
#define LED3 19

unsigned long ulTimer;
uint8_t currentLED = 0;

const uint32_t ledInterval[] = {
  10000, // LED1
  5000,  // LED2
  3000   // LED3
};

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  digitalWrite(LED1, HIGH);   
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  ulTimer = millis();

  printf("LED1 ON (10s)\n");
}

void loop() {
  if (millis() - ulTimer >= ledInterval[currentLED]) {
    ulTimer = millis();

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);

    currentLED++;
    if (currentLED > 2) currentLED = 0;

    if (currentLED == 0) {
      digitalWrite(LED1, HIGH);
      printf("LED1 ON (10s)\n");
    }
    else if (currentLED == 1) {
      digitalWrite(LED2, HIGH);
      printf("LED2 ON (5s)\n");
    }
    else {
      digitalWrite(LED3, HIGH);
      printf("LED3 ON (3s)\n");
    }
  }
}
