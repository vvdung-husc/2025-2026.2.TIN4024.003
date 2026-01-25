#pragma once
#include <Arduino.h>

bool IsReady(unsigned long &ulTimer, uint32_t millisecond);
String StringFormat(const char* fmt, ...);  