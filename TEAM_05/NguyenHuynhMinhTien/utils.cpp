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