#include <Arduino.h>
#include <TM1637Display.h>

/* ===================== TIỆN ÍCH ===================== */
bool IsReady(unsigned long &timer, uint32_t ms)
{
  if (millis() - timer < ms) return false;
  timer = millis();
  return true;
}

/* ===================== KHAI BÁO CHÂN ===================== */
// LED giao thông
#define PIN_LED_RED     25
#define PIN_LED_YELLOW  33
#define PIN_LED_GREEN   32

// LED trạng thái
#define PIN_LED_BLUE    21

// Nút nhấn
#define PIN_BUTTON      23

// LDR
#define PIN_LDR         13

// TM1637
#define CLK 15
#define DIO 2

TM1637Display display(CLK, DIO);

/* ===================== NGƯỠNG ÁNH SÁNG ===================== */
// > 2200 ~ rất tối (~ < 80 lux)
#define NIGHT_LDR_THRESHOLD 2200

/* ===================== BIẾN HỆ THỐNG ===================== */
bool isPause = false;

/* ===================== KHỞI TẠO LED ===================== */
void InitLED()
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
    Serial.println(isPause ? "SYSTEM PAUSE" : "SYSTEM RUN");
  }

  lastState = currentState;
}

/* ===================== ĐỌC LDR ===================== */
uint16_t ReadLDR()
{
  static unsigned long ldrTimer = 0;
  static uint16_t value = 0;

  if (IsReady(ldrTimer, 500))
  {
    value = analogRead(PIN_LDR);
    Serial.print("LDR Value: ");
    Serial.println(value);
  }

  return value;
}

/* ===================== NIGHT MODE ===================== */
// Trời rất tối → đèn đỏ nhấp nháy
void NightModeBlinkRed()
{
  static unsigned long blinkTimer = 0;
  static bool state = false;

  if (!IsReady(blinkTimer, 500)) return;

  state = !state;

  digitalWrite(PIN_LED_RED, state);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  display.showNumberDec(0, true); // 0000
}

/* ===================== ĐÈN GIAO THÔNG BÌNH THƯỜNG ===================== */
void TrafficLightNormal()
{
  static unsigned long timer = 0;
  static uint8_t step = 0;
  static int countDown = 0;

  const uint8_t LEDs[3] = {
    PIN_LED_GREEN,
    PIN_LED_YELLOW,
    PIN_LED_RED
  };

  const uint8_t times[3] = {
    7, // xanh 7s
    3, // vàng 3s
    5  // đỏ 5s
  };

  if (!IsReady(timer, 1000)) return;

  if (countDown == 0)
  {
    // Tắt tất cả LED
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_GREEN, LOW);

    // Bật LED hiện tại
    digitalWrite(LEDs[step], HIGH);
    countDown = times[step];
    step = (step + 1) % 3;
  }

  display.showNumberDec(countDown, true);
  countDown--;
}

/* ===================== SETUP ===================== */
void setup()
{
  Serial.begin(115200);
  Serial.println("\n=== TRAFFIC LIGHT + LDR SYSTEM ===");

  InitLED();

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_LDR, INPUT);

  display.setBrightness(0x0a);
  display.clear();
}

/* ===================== LOOP ===================== */
void loop()
{
  ProcessButton();

  uint16_t ldrValue = ReadLDR();

  // 🌙 Trời rất tối → NIGHT MODE
  if (ldrValue > NIGHT_LDR_THRESHOLD)
  {
    digitalWrite(PIN_LED_BLUE, HIGH);
    NightModeBlinkRed();
    return;
  }

  // ☀️ Trời sáng → đèn giao thông bình thường
  digitalWrite(PIN_LED_BLUE, isPause ? HIGH : LOW);

  if (!isPause)
  {
    TrafficLightNormal();
  }
}
