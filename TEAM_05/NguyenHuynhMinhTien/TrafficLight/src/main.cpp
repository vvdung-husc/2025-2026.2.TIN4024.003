#include <Arduino.h>
#include <TM1637Display.h>

// Hàm kiểm tra thời gian đã trôi qua - Non-Blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}
// Định dạng chuỗi %s,%d,...
String StringFormat(const char *fmt, ...)
{
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char *buff = (char *)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// Module connection pins (Digital Pins)
#define CLK 15
#define DIO 2

#define PIN_BUTTON_DISPLAY 23
#define PIN_LED_BLUE      21

TM1637Display display(CLK, DIO);
int valueButtonDisplay = LOW;

const char* LEDString(uint8_t pin)
{
  switch (pin)
  {
    case PIN_LED_RED:     return "RED";
    case PIN_LED_YELLOW:  return "YELLOW";
    case PIN_LED_GREEN:   return "GREEN";
    default:              return "UNKNOWN";
  }
}

void Init_LED_Traffic()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
}

bool ProcessLEDTraffic()
{
  static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0;
  static uint8_t LEDs[3] = {PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED};
  if (!IsReady(ulTimer, 1000)) return false;

  for (size_t i = 0; i < 3; i++)
  {
    if (i == idxLED) digitalWrite(LEDs[i], HIGH);
    else digitalWrite(LEDs[i], LOW);
  }

  idxLED = (idxLED + 1) % 3;// Next LED => idxLED = 0,1,2,...

  return true;
}

bool ProcessLEDTrafficWaitTime()
{
  static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0;//PIN_LED_GREEN
  static uint8_t LEDs[3] = {PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED};
  static uint32_t waitTime[3] = {7000, 3000, 5000};// Green, Yellow, Red
  static uint32_t count = waitTime[idxLED];
  static bool ledStatus = false;
  static int secondCount = 0;

  if (!IsReady(ulTimer, 500)) return false;

  if (count == waitTime[idxLED])
  {
    secondCount = (count / 1000) - 1;

    ledStatus = true;
    for (size_t i = 0; i < 3; i++)
    {
      if (i == idxLED){
        digitalWrite(LEDs[i], HIGH);
        printf("LED [%-6s] ON => %d Seconds\n", LEDString(LEDs[i]), count/1000);
      }
      else digitalWrite(LEDs[i], LOW);
    }
  }
  else {
    ledStatus = !ledStatus;
    digitalWrite(LEDs[idxLED], ledStatus ? HIGH : LOW);
  }

  if (ledStatus) {
    if (valueButtonDisplay == HIGH){
       printf(" [%s] => seconds: %d \n",LEDString(LEDs[idxLED]), secondCount);
       display.showNumberDec(secondCount);
    }
    --secondCount;
  }

  count -= 500;
  if (count > 0) return true;

  idxLED = (idxLED + 1) % 3;// Next LED => idxLED = 0,1,2,...
  count = waitTime[idxLED];

  return true;
}

void ProcessButtonPressed(){
  static ulong ulTimer = 0;

  if (!IsReady(ulTimer, 10)) return;

  int newValue = digitalRead(PIN_BUTTON_DISPLAY);
  if (newValue == valueButtonDisplay) return;

  if (newValue == HIGH){
    digitalWrite(PIN_LED_BLUE, HIGH);
    printf("*** DISPLAY ON ***\n");
  }
  else {
    digitalWrite(PIN_LED_BLUE, LOW);
    display.clear();
    printf("*** DISPLAY OFF ***\n");
  }

  valueButtonDisplay = newValue;
}

void setup()
{
  // put your setup code here, to run once:
  printf("*** PROJECT LED TRAFFIC ***\n");
  Init_LED_Traffic();

  display.setBrightness(0x0a);
  display.clear();

  pinMode(PIN_BUTTON_DISPLAY, INPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
}


void loop()
{
  // put your main code here, to run repeatedly:
  //ProcessLEDTraffic();
  ProcessButtonPressed();
  ProcessLEDTrafficWaitTime();
}