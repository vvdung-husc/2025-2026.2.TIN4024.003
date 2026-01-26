#include <Arduino.h>

const uint8_t RedLedPin = 00;
const uint8_t YellowLedPin = 02;
const uint8_t GreenLedPin = 04;

uint8_t lightState = 0;

const unsigned long redInterval = 10000;    // 10 seconds
const unsigned long yellowInterval = 3000;  // 3 seconds
const unsigned long greenInterval = 5000;   // 5 seconds


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
    
    unsigned long currentInterval;
    if (lightState == 0)
    {
        currentInterval = redInterval;
    }
    else if (lightState == 1)
    {
        currentInterval = yellowInterval;
    }
    else
    {
        currentInterval = greenInterval;
    }

    if (isReady(currentInterval))
    {
        digitalWrite(RedLedPin, LOW);
        digitalWrite(YellowLedPin, LOW);
        digitalWrite(GreenLedPin, LOW);

        lightState = (lightState + 1) % 3;

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