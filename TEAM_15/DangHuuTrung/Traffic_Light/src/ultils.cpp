#include "ultils.h"

// ================= LED =================
LED::LED()
{
    _pin = -1;
    _state = false;
    _previousMillis = 0;
}

void LED::setup(int pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

void LED::blink(unsigned long interval)
{
    if (!IsReady(_previousMillis, interval)) return;
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
}

void LED::set(bool bON)
{
    digitalWrite(_pin, bON ? HIGH : LOW);
}

// ================= LDR =================
int LDR::DAY_THRESHOLD = 2000;

LDR::LDR()
{
    _pin = -1;
    _value = 0;
    _vcc5Volt = true;
}

void LDR::setup(int pin, bool vcc5Volt)
{
    _pin = pin;
    _vcc5Volt = vcc5Volt;
    pinMode(_pin, INPUT);
}

int LDR::getValue()
{
    _value = analogRead(_pin);
    return _value;
}

// ================= TRAFIC =================
Trafic_Blink::Trafic_Blink()
{
    _ledStatus = false;
    _previousMillis = 0;
    _idxLED = 0;
    _secondCount = 0;
    _displayOn = false;
}

void Trafic_Blink::setupPin(int pinRed, int pinYellow, int pinGreen, int pinBlue, int pinButton)
{
    _LEDs[0] = pinGreen;
    _LEDs[1] = pinYellow;
    _LEDs[2] = pinRed;

    pinMode(pinRed, OUTPUT);
    pinMode(pinYellow, OUTPUT);
    pinMode(pinGreen, OUTPUT);

    ledBlue.setup(pinBlue);

    _pinButton = pinButton;
    pinMode(_pinButton, INPUT_PULLUP); // 🔥 FIX FLOAT
}

void Trafic_Blink::setupWaitTime(uint32_t redWait, uint32_t yellowWait, uint32_t greenWait)
{
    _waitTime[0] = greenWait * 1000;
    _waitTime[1] = yellowWait * 1000;
    _waitTime[2] = redWait * 1000;
}

// 🔥 EDGE DETECT + DEBOUNCE (INPUT_PULLUP)
bool Trafic_Blink::handleButton()
{
    static bool lastState = HIGH;
    static unsigned long lastDebounce = 0;

    bool current = digitalRead(_pinButton);

    if (current != lastState)
    {
        lastDebounce = millis();
    }

    if ((millis() - lastDebounce) > 30)
    {
        // HIGH → LOW = NHẤN
        if (lastState == HIGH && current == LOW)
        {
            lastState = current;
            return true;
        }
    }

    lastState = current;
    return false;
}

void Trafic_Blink::blink(LDR& ldrSensor, TM1637Display& display)
{
    static uint32_t count = 0;

    // xử lý nút nhanh
    if (handleButton())
    {
        _displayOn = !_displayOn;
        ledBlue.set(_displayOn);

        if (_displayOn)
            printf("*** DISPLAY ON ***\n");
        else
        {
            display.clear();
            printf("*** DISPLAY OFF ***\n");
        }
    }

    if (!IsReady(_previousMillis, 500)) return;

    if (count == 0)
        count = _waitTime[_idxLED];

    digitalWrite(_LEDs[0], LOW);
    digitalWrite(_LEDs[1], LOW);
    digitalWrite(_LEDs[2], LOW);

    digitalWrite(_LEDs[_idxLED], HIGH);

    if (_displayOn)
        display.showNumberDec(count / 1000);

    count -= 500;

    if (count <= 0)
        _idxLED = (_idxLED + 1) % 3;
}

// ================= UTILS =================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
}
