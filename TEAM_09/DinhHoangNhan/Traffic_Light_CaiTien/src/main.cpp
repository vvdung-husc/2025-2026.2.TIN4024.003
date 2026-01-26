#include <Arduino.h>
#include <TM1637Display.h>

// ======================= PIN MAP =======================
// Traffic LEDs
#define LED_RED 25
#define LED_YELLOW 26
#define LED_GREEN 27

// Street light LED (controlled by LDR)
#define LED_STREET 33

// Push button: toggle display ON/OFF
#define BTN_TOGGLE 14 // dùng PULLUP, nút nối về GND

// TM1637
#define TM_CLK 18
#define TM_DIO 19
TM1637Display display(TM_CLK, TM_DIO);

// LDR analog pin (ESP32 ADC)
#define LDR_AO 34

// ======================= CONFIG =======================
// Durations (ms)
const uint32_t DUR_RED = 2000;
const uint32_t DUR_YELLOW = 3000;
const uint32_t DUR_GREEN = 5000;

// Blink interval for active traffic LED
const uint32_t BLINK_INTERVAL = 200; // ms

// LDR threshold (0..4095). Tối < threshold => bật đèn đường
// Bạn có thể chỉnh 1500/2000 tùy cảm nhận
const int LDR_THRESHOLD = 1800;

// ======================= STATE =======================
enum Phase
{
  PHASE_RED,
  PHASE_YELLOW,
  PHASE_GREEN
};
Phase currentPhase = PHASE_RED;

uint32_t phaseStartMs = 0;
uint32_t phaseDurationMs = DUR_RED;

// blink
uint32_t lastBlinkMs = 0;
bool blinkState = false;

// countdown
uint32_t lastCountdownMs = 0;
int lastShownSeconds = -1;

// display toggle
bool displayEnabled = true;

// button debounce
bool lastBtnReading = HIGH;
bool btnStableState = HIGH;
uint32_t lastDebounceMs = 0;
const uint32_t DEBOUNCE_MS = 35;

// ======================= HELPERS =======================
void allTrafficOff()
{
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

int activeLedPin()
{
  switch (currentPhase)
  {
  case PHASE_RED:
    return LED_RED;
  case PHASE_YELLOW:
    return LED_YELLOW;
  case PHASE_GREEN:
    return LED_GREEN;
  }
  return LED_RED;
}

uint32_t activeDuration()
{
  switch (currentPhase)
  {
  case PHASE_RED:
    return DUR_RED;
  case PHASE_YELLOW:
    return DUR_YELLOW;
  case PHASE_GREEN:
    return DUR_GREEN;
  }
  return DUR_RED;
}

void nextPhase()
{
  if (currentPhase == PHASE_RED)
    currentPhase = PHASE_YELLOW;
  else if (currentPhase == PHASE_YELLOW)
    currentPhase = PHASE_GREEN;
  else
    currentPhase = PHASE_RED;

  phaseStartMs = millis();
  phaseDurationMs = activeDuration();

  // reset blink
  blinkState = false;
  lastBlinkMs = 0;

  // reset countdown display update
  lastShownSeconds = -1;
}

void updateTrafficBlink()
{
  uint32_t now = millis();
  if (now - lastBlinkMs >= BLINK_INTERVAL)
  {
    lastBlinkMs = now;
    blinkState = !blinkState;

    allTrafficOff(); // đảm bảo chỉ 1 đèn hoạt động
    digitalWrite(activeLedPin(), blinkState ? HIGH : LOW);
  }
}

void updateCountdownDisplay()
{
  // nếu tắt hiển thị thì clear và không update nữa
  if (!displayEnabled)
  {
    display.clear();
    return;
  }

  uint32_t now = millis();
  if (now - lastCountdownMs < 200)
    return; // update vừa đủ mượt
  lastCountdownMs = now;

  uint32_t elapsed = now - phaseStartMs;
  int remainMs = (int)phaseDurationMs - (int)elapsed;
  if (remainMs < 0)
    remainMs = 0;

  // làm tròn lên thành giây còn lại (vd: 1.2s -> 2s)
  int remainSeconds = (remainMs + 999) / 1000;

  if (remainSeconds != lastShownSeconds)
  {
    lastShownSeconds = remainSeconds;

    // Hiển thị 0..9999 (ở đây tối đa 5 giây)
    // Nếu muốn đẹp hơn: hiển thị dạng 00:05 => dùng showNumberDecEx
    // Nhưng đề bài chỉ cần “thời gian” nên hiển thị số giây là ok.
    display.showNumberDec(remainSeconds, true, 4, 0);
  }
}

void updateButtonToggle()
{
  bool reading = digitalRead(BTN_TOGGLE);

  if (reading != lastBtnReading)
  {
    lastDebounceMs = millis();
    lastBtnReading = reading;
  }

  if (millis() - lastDebounceMs > DEBOUNCE_MS)
  {
    if (reading != btnStableState)
    {
      btnStableState = reading;

      // Nhấn nút (PULLUP => nhấn là LOW)
      if (btnStableState == LOW)
      {
        displayEnabled = !displayEnabled;

        if (!displayEnabled)
          display.clear();
        // nếu bật lại thì cập nhật ngay
        lastShownSeconds = -1;
      }
    }
  }
}

void updateLdrStreetLight()
{
  int ldrValue = analogRead(LDR_AO); // 0..4095
  bool isDark = (ldrValue < LDR_THRESHOLD);

  digitalWrite(LED_STREET, isDark ? HIGH : LOW);
}

// ======================= SETUP/LOOP =======================
void setup()
{
  // pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_STREET, OUTPUT);

  pinMode(BTN_TOGGLE, INPUT_PULLUP);

  allTrafficOff();
  digitalWrite(LED_STREET, LOW);

  // display
  display.setBrightness(7, true); // 0..7
  display.clear();

  // start phase
  currentPhase = PHASE_RED;
  phaseStartMs = millis();
  phaseDurationMs = activeDuration();
}

void loop()
{
  // 1) đọc nút bật/tắt hiển thị
  updateButtonToggle();

  // 2) LDR điều khiển đèn đường
  updateLdrStreetLight();

  // 3) nháy đèn giao thông theo pha hiện tại
  updateTrafficBlink();

  // 4) hiển thị đếm ngược
  updateCountdownDisplay();

  // 5) chuyển pha khi hết thời gian
  uint32_t now = millis();
  if (now - phaseStartMs >= phaseDurationMs)
  {
    nextPhase();
  }
}
