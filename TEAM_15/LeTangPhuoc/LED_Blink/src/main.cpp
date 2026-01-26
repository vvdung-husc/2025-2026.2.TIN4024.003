#include <Arduino.h>

uint8_t PIN_LED_RED     = 18;
uint8_t PIN_LED_YELLOW  = 5;
uint8_t PIN_LED_BLUE    = 17;

//Non-blocking
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


void setup() {
  // put your setup code here, to run once:
  printf("Welcome IoT\n");
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT); 
}

unsigned long ulTimer = 0;
bool bLEDStatus = false;

void RED_Blink(){
  if (IsReady(ulTimer,1000)){
    bLEDStatus = !bLEDStatus;
    digitalWrite(PIN_LED_RED, bLEDStatus); 
  }
}

uint8_t LEDs[3] = {PIN_LED_RED, PIN_LED_YELLOW, PIN_LED_BLUE};
uint8_t idxLED = 0; //PIN_LED_RED
uint8_t delaySeconds = 0;

void LightBlink(uint8_t idx){
    if (IsReady(ulTimer,1000)){
    bLEDStatus = !bLEDStatus;
    digitalWrite(LEDs[idx], bLEDStatus); 
  }
}

void Traffic_Light(){

}

void loop() {
  //RED_Blink();
  Traffic_Light();
}

// void loop() {
//   // put your main code here, to run repeatedly:
//   printf("[LED_RED] => HIGH\n");
//   digitalWrite(PIN_LED_RED, HIGH); // Turn LED ON
//   delay(500); // Wait for 500 milliseconds
//   printf("[LED_RED] => LOW\n");
//   digitalWrite(PIN_LED_RED, LOW); // Turn LED OFF
//   delay(500); // Wait for 500 milliseconds  
// }


