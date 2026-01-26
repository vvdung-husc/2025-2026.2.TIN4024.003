#include <Arduino.h>

// Pin definitions (from updated diagram)
const uint8_t RED_LED = 27;
const uint8_t YELLOW_LED = 23;
const uint8_t GREEN_LED = 5;

// Function: blink selected light for given duration (seconds)
// Blink interval: 500 ms
void blinkLight(uint8_t lightPin, int durationSeconds)
{
  unsigned long startTime = millis();
  unsigned long durationMs = durationSeconds * 1000UL;

  while (millis() - startTime < durationMs)
  {
    digitalWrite(lightPin, HIGH);
    delay(500);
    digitalWrite(lightPin, LOW);
    delay(500);
  }
}

void setup()
{
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Ensure all LEDs are off
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void loop()
{
  // Red → 5 seconds
  blinkLight(RED_LED, 5);

  // Green → 5 seconds
  blinkLight(GREEN_LED, 5);

  // Yellow → 3 seconds
  blinkLight(YELLOW_LED, 3);
}
