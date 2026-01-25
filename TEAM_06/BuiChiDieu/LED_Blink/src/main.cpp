#include <Arduino.h>

const uint8_t RedLedPin = 00;
const uint8_t YellowLedPin = 02;
const uint8_t GreenLedPin = 04;

uint8_t lightState = 0;


bool isReady(unsigned long interval)
{
    static unsigned long previousMillis = 0;

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        return true;
    }

    return false;
}

void setup()
{
    pinMode(RedLedPin, OUTPUT);
    pinMode(YellowLedPin, OUTPUT);
    pinMode(GreenLedPin, OUTPUT);

    digitalWrite(RedLedPin, HIGH);
    digitalWrite(YellowLedPin, LOW);
    digitalWrite(GreenLedPin, LOW);
}

void loop()
{
    if (isReady(3000))
    {

        digitalWrite(RedLedPin, LOW);
        digitalWrite(YellowLedPin, LOW);
        digitalWrite(GreenLedPin, LOW);

        // Move to next state
        lightState = (lightState + 1) % 3;

        // Turn on the appropriate LED
        if (lightState == 0)
        {
            digitalWrite(RedLedPin, HIGH);
        }
        else if (lightState == 1)
        {
            digitalWrite(YellowLedPin, HIGH);
        }
        else
        {
            digitalWrite(GreenLedPin, HIGH);
        }
    }
}