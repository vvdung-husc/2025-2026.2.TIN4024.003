#include <Arduino.h>
#include <TM1637Display.h>

// ================= PIN =================
#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define BUTTON_PIN  23
#define LDR_PIN     13     // cảm biến ánh sáng

#define CLK 18
#define DIO 19

#define NIGHT_THRESHOLD 500   // chỉnh nếu cần

TM1637Display display(CLK, DIO);

// ================= TIMER =================
unsigned long stateTimer = 0;
unsigned long secondTimer = 0;
unsigned long blinkTimer = 0;

// ================= FLAG =================
bool showDisplay = true;
bool nightMode = false;

// ================= STATE =================
enum TrafficState { RED, GREEN, YELLOW };
TrafficState state = RED;

// thời gian từng trạng thái (giây)
int countdown = 5;

// ================= NON BLOCK =================
bool IsReady(unsigned long &t, uint32_t ms)
{
  if (millis() - t < ms) return false;
  t = millis();
  return true;
}

// ================= LIGHT =================
void SetLight(bool r, bool y, bool g)
{
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  SetLight(1,0,0);
  display.showNumberDec(countdown, true);

  stateTimer = millis();
  secondTimer = millis();

  Serial.println("Traffic light started");
}

// ================= LOOP =================
void loop()
{
  // ================= CHECK NIGHT =================
  int ldr = analogRead(LDR_PIN);
  nightMode = ldr < NIGHT_THRESHOLD;

  // ================= NIGHT MODE =================
  if (nightMode)
  {
    display.clear();

    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);

    // vàng nhấp nháy
    if (IsReady(blinkTimer, 500))
      digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));

    return; // bỏ toàn bộ xử lý ban ngày
  }

  // đảm bảo vàng tắt khi trở lại ban ngày
  digitalWrite(LED_YELLOW, LOW);

  // -------- BUTTON toggle display --------
  static bool lastBtn = HIGH;
  bool btn = digitalRead(BUTTON_PIN);

  if (lastBtn == HIGH && btn == LOW)
  {
    showDisplay = !showDisplay;

    if (!showDisplay)
      display.clear();
    else
      display.showNumberDec(countdown, true);

    delay(200);
  }
  lastBtn = btn;

  // -------- COUNTDOWN LOGIC --------
  if (IsReady(secondTimer, 1000))
  {
    countdown--;
    if (countdown < 0) countdown = 0;

    if (showDisplay)
      display.showNumberDec(countdown, true);
  }

  // -------- TRAFFIC STATE --------
  switch (state)
  {
    case RED:
      SetLight(1,0,0);

      if (IsReady(stateTimer, 5000))
      {
        state = GREEN;
        countdown = 5;
        if (showDisplay) display.showNumberDec(countdown, true);
      }
      break;

    case GREEN:
      SetLight(0,0,1);

      if (IsReady(stateTimer, 5000))
      {
        state = YELLOW;
        countdown = 2;
        if (showDisplay) display.showNumberDec(countdown, true);
      }
      break;

    case YELLOW:
      SetLight(0,1,0);

      if (IsReady(stateTimer, 2000))
      {
        state = RED;
        countdown = 5;
        if (showDisplay) display.showNumberDec(countdown, true);
      }
      break;
  }
}
