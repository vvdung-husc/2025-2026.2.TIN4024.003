#include "utils.h"

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
    if (millis() - ulTimer < millisecond) return false;
    ulTimer = millis();
    return true;
  }
  //Định dạng chuỗi %s,%d,...
  String StringFormat(const char* fmt, ...){
    va_list vaArgs;
    va_start(vaArgs, fmt);
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
    va_end(vaArgsCopy);
    int iSize = iLen + 1;
    char* buff = (char*)malloc(iSize);
    vsnprintf(buff, iSize, fmt, vaArgs);
    va_end(vaArgs);
    String s = buff;
    free(buff);
    return String(s);
  }

int LDR::DAY_THRESHOLD = 2000;
LDR::LDR()
{
    _pin = -1;
    _value = 0;
    _vcc5Volt = false;
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

float LDR::readLux(int *analogValue)
{
    static float prevLux = -1.0;
    float voltage, resistance, lux;

    getValue();

    if (analogValue != nullptr)
    {
        *analogValue = _value;
    }

    if (_vcc5Volt)
    {
        // VCC = 5V
        voltage = (float)_value * 5.0 / 4095.0;
        resistance = 2000 * voltage / (1 - voltage / 5.0);
        lux = pow(50 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
    }
    else
    {
        // VCC = 3.3V
        voltage = (float)_value * 3.3 / 4095.0;
        resistance = 2000 * voltage / (1 - voltage / 3.3);
        lux = pow(33 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
    }

    if (lux != prevLux)
    {
        prevLux = lux;
        if (prevLux >= 1.0)
            printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.0f] lux\n", _value, voltage, resistance, lux, prevLux);
        else
            printf("LDR Analog: %d, Voltage: %.2f V, Resistance: %.2f Ohm, Light Intensity: %.2f[%.1f] lux\n", _value, voltage, resistance, lux, prevLux);
    }

    return lux;
}