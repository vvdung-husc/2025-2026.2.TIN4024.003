#include <Arduino.h>
#include <TM1637Display.h>
#include <stdarg.h>

// =====================
// NON-BLOCKING TIMER
// =====================
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

// =====================
// STRING FORMAT
// =====================
String StringFormat(const char *fmt, ...)
{
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  char *buff = (char *)malloc(iLen + 1);
  vsnprintf(buff, iLen + 1, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return s;
}

// =====================
// PIN DEFINE
// =====================
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32
#define PIN_BUTTON      23

#define CLK 15
#define DIO 2

TM1637Display display(CLK, DIO);

// =====================
// GLOBAL STATE
// =====================
bool systemEnable = true;

// =====================
// HELPER
// =====================
const char* LEDString(uint8_t pin)
{
  switch (pin)
  {
    case PIN_LED_RED:    return "RED";
    case PIN_LED_YELLOW: return "YELLOW";
    case PIN_LED_GREEN:  return "GREEN";
    default:             return "UNKNOWN";
  }
}

// =====================
// INIT
// =====================
void Init_LED_Traffic()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
}

// =====================
// BUTTON PROCESS (TOGGLE ON/OFF)
// =====================
void ProcessButton()
{
  static unsigned long btnTimer = 0;
  static bool lastState = HIGH;

  bool currentState = digitalRead(PIN_BUTTON);

  if (lastState == HIGH && currentState == LOW)
  {
    if (IsReady(btnTimer, 300))   // debounce
    {
      systemEnable = !systemEnable;
      printf("SYSTEM %s\n", systemEnable ? "ON" : "OFF");
    }
  }
  lastState = currentState;
}

// =====================
// TRAFFIC LIGHT WITH TIME
// =====================
bool ProcessLEDTrafficWaitTime()
{
  static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0;
  static uint8_t LEDs[3] = { PIN_LED_GREEN, PIN_LED_YELLOW, PIN_LED_RED };
  static uint32_t waitTime[3] = { 7000, 3000, 5000 };
  static uint32_t count = waitTime[0];
  static bool ledStatus = false;
  static int secondCount = 0;

  if (!IsReady(ulTimer, 500)) return false;

  if (count == waitTime[idxLED])
  {
    secondCount = (count / 1000) - 1;
    ledStatus = true;

    for (int i = 0; i < 3; i++)
      digitalWrite(LEDs[i], (i == idxLED) ? HIGH : LOW);

    printf("LED [%s] ON => %d Seconds\n",
           LEDString(LEDs[idxLED]), count / 1000);
  }
  else
  {
    ledStatus = !ledStatus;
    digitalWrite(LEDs[idxLED], ledStatus ? HIGH : LOW);
  }

  if (ledStatus)
  {
    display.showNumberDec(secondCount);
    printf("[%s] => seconds: %d\n", LEDString(LEDs[idxLED]), secondCount);
    secondCount--;
  }

  count -= 500;
  if (count > 0) return true;

  idxLED = (idxLED + 1) % 3;
  count = waitTime[idxLED];
  return true;
}

// =====================
// SETUP
// =====================
void setup()
{
  printf("*** PROJECT LED TRAFFIC ***\n");

  Init_LED_Traffic();
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  display.setBrightness(0x0a);
}

// =====================
// LOOP
// =====================
void loop()
{
  ProcessButton();

  if (systemEnable)
  {
    ProcessLEDTrafficWaitTime();
  }
  else
  {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);
    display.clear();
  }
}
