#pragma once
#include <Arduino.h>

bool IsReady(unsigned long &ulTimer, uint32_t millisecond);
String StringFormat(const char* fmt, ...);

class LDR
{
public:
    static int DAY_THRESHOLD;
public:
    LDR();
    void setup(int pin, bool vcc5Volt = false); // VCC = 3.3V or 5V
    int getValue(); // Analog value 0-4095
    float readLux(int* analogValue = nullptr); // Return light intensity in lux
    
private:
    int _pin;
    int _value;
    bool _vcc5Volt;
};  