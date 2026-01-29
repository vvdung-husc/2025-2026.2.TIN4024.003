#pragma once
#include <Arduino.h>

class LED_Blink
{
public:
    LED_Blink();
    void setup(int pin);
    void blink(unsigned long interval = 500);

private:
    int _pin;
    bool _state;
    unsigned long _previousMillis;
};

class Trafic_Blink
{
public:
    Trafic_Blink();
    void setupPin(int pinRed, int pinYellow, int pinGreen);
    void setupWaitTime(uint32_t redWait = 5, uint32_t yellowWait = 3, uint32_t greenWait = 10); // seconds
    void blink(unsigned long interval = 500, bool isDark = false); //miliseconds
    const char *ledString(int pin);

private:
    bool _ledStatus;
    unsigned long _previousMillis;
    int _LEDs[3];
    int _idxLED;
    uint32_t _waitTime[3];
};

class LDR
{
public:
    LDR();
    void setup(int pin, bool vcc5Volt = true); // VCC = 3.3V or 5V
    int getValue(); // Analog value 0-4095
    float readLux(int* analogValue = nullptr); // Return light intensity in lux

private:
    int _pin;
    int _value;
    bool _vcc5Volt;
};

bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 1000);

// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...);