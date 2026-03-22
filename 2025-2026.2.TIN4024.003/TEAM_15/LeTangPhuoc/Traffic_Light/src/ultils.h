#pragma once
#include <Arduino.h>
#include <TM1637Display.h>

// ================= LED =================
class LED
{
public:
    LED();
    void setup(int pin);
    void blink(unsigned long interval = 500);
    void set(bool bON);

private:
    int _pin;
    bool _state;
    unsigned long _previousMillis;
};

// ================= LDR =================
class LDR
{
public:
    static int DAY_THRESHOLD;

    LDR();
    void setup(int pin, bool vcc5Volt = true);
    int getValue();

private:
    int _pin;
    int _value;
    bool _vcc5Volt;
};

// ================= TRAFIC =================
class Trafic_Blink
{
public:
    Trafic_Blink();
    void setupPin(int pinRed, int pinYellow, int pinGreen, int pinBlue, int pinButton);
    void setupWaitTime(uint32_t redWait = 5, uint32_t yellowWait = 3, uint32_t greenWait = 10);
    void blink(LDR& ldrSensor, TM1637Display& display);

private:
    // traffic
    bool _ledStatus;
    unsigned long _previousMillis;
    int _LEDs[3];
    int _idxLED;
    uint32_t _waitTime[3];
    int _secondCount;

    // button + display
    int _pinButton;
    bool _displayOn;

    // blue led
    LED ledBlue;

    // helpers
    bool handleButton();   // 🔥 FIX CHUẨN
};

// ================= UTILS =================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond = 1000);
