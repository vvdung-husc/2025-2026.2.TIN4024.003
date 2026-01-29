#include <Arduino.h>
#include <TM1637Display.h>

/* ===================== TIỆN ÍCH ===================== */
// Hàm kiểm tra thời gian non-blocking
bool IsReady(unsigned long &ulTimer, uint32_t millisecond)
{
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

/* ===================== KHAI BÁO CHÂN ===================== */
// LED giao thông
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// LED trạng thái PAUSE
#define PIN_LED_BLUE    21

// Nút nhấn
#define PIN_BUTTON      23

// TM1637
#define CLK 15
#define DIO 2

TM1637Display display(CLK, DIO);

/* ===================== BIẾN HỆ THỐNG ===================== */
bool isPause = false;

/* ===================== TIỆN ÍCH HIỂN THỊ ===================== */
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

/* ===================== KHỞI TẠO ===================== */
void Init_LED_Traffic()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);
}

/* ===================== XỬ LÝ NÚT NHẤN ===================== */
void ProcessButton()
{
  static unsigned long btnTimer = 0;
  static bool lastState = HIGH;

  if (!IsReady(btnTimer, 50)) return;

  bool currentState = digitalRead(PIN_BUTTON);

  if (lastState == HIGH && currentState == LOW)
  {
    isPause = !isPause;
    printf("SYSTEM %s\n", isPause ? "PAUSE" : "RUN");
    digitalWrite(PIN_LED_BLUE, isPause ? HIGH : LOW);
  }

  lastState = currentState;
}

/* ===================== ĐÈN GIAO THÔNG + ĐẾM NGƯỢC ===================== */
bool ProcessLEDTrafficWaitTime()
{
  static unsigned long ulTimer = 0;
  static uint8_t idxLED = 0;

  static uint8_t LEDs[3] = {
    PIN_LED_GREEN,
    PIN_LED_YELLOW,
    PIN_LED_RED
  };

  static uint32_t waitTime[3] = {
    7000, // GREEN
    3000, // YELLOW
    5000  // RED
  };

  static uint32_t count = waitTime[0];
  static bool ledStatus = false;
  static int secondCount = 0;

  if (!IsReady(ulTimer, 500)) return false;

  // Bắt đầu chu kỳ LED mới
  if (count == waitTime[idxLED])
  {
    secondCount = (count / 1000) - 1;
    ledStatus = true;

    for (int i = 0; i < 3; i++)
    {
      digitalWrite(LEDs[i], (i == idxLED) ? HIGH : LOW);
    }

    printf("LED [%s] ON (%d seconds)\n",
           LEDString(LEDs[idxLED]), count / 1000);
  }
  else
  {
    ledStatus = !ledStatus;
    digitalWrite(LEDs[idxLED], ledStatus ? HIGH : LOW);
  }

  // Hiển thị đếm ngược
  if (ledStatus)
  {
    display.showNumberDec(secondCount, true);
    printf("[%s] remaining: %d\n",
           LEDString(LEDs[idxLED]), secondCount);
    secondCount--;
  }

  count -= 500;
  if (count > 0) return true;

  // Chuyển LED tiếp theo
  idxLED = (idxLED + 1) % 3;
  count = waitTime[idxLED];

  return true;
}

/* ===================== SETUP ===================== */
void setup()
{
  Serial.begin(115200);
  printf("\n*** LED TRAFFIC SYSTEM ***\n");

  Init_LED_Traffic();

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED_BLUE, OUTPUT);
  digitalWrite(PIN_LED_BLUE, LOW);

  display.setBrightness(0x0a);
  display.clear();
}

/* ===================== LOOP ===================== */
void loop()
{
  ProcessButton();

  if (!isPause)
  {
    ProcessLEDTrafficWaitTime();
  }
}