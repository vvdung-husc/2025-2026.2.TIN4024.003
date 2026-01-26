#include <Arduino.h>
#include <TM1637Display.h>

// ================= PIN =================

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ================= TIMER =================

unsigned long stateTimer = 0;
unsigned long secondTimer = 0;

// ================= STATE =================

enum TrafficState
{
  RED,
  GREEN,
  YELLOW
};

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

  display.setBrightness(7);

  // bắt đầu RED
  SetLight(1, 0, 0);

  countdown = 5;
  display.showNumberDec(countdown, true);

  stateTimer = millis();
  secondTimer = millis();

  Serial.println("Traffic light started");
}

// ================= LOOP =================

void loop()
{
  // --------- Countdown mỗi giây ---------

  if (IsReady(secondTimer, 1000))
  {
    countdown--;
    if (countdown < 0) countdown = 0;
    display.showNumberDec(countdown, true);
  }

  // --------- State machine ---------

  switch (state)
  {
    case RED:
      SetLight(1, 0, 0);

      if (IsReady(stateTimer, 5000))
      {
        state = GREEN;
        countdown = 5;
        display.showNumberDec(countdown, true);
        Serial.println("GREEN");
      }
      break;

    case GREEN:
      SetLight(0, 0, 1);

      if (IsReady(stateTimer, 5000))
      {
        state = YELLOW;
        countdown = 2;
        display.showNumberDec(countdown, true);
        Serial.println("YELLOW");
      }
      break;

    case YELLOW:
      SetLight(0, 1, 0);

      if (IsReady(stateTimer, 2000))
      {
        state = RED;
        countdown = 5;
        display.showNumberDec(countdown, true);
        Serial.println("RED");
      }
      break;
  }
}
