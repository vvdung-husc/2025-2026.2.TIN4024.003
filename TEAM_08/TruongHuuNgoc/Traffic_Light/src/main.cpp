#include <Arduino.h>
#include <TM1637Display.h>

// ================= PIN =================

#define LED_RED     27
#define LED_YELLOW  26
#define LED_GREEN   25
#define LED_BLUE    21

#define BUTTON_PIN  23
#define LDR_PIN     13

#define CLK 18
#define DIO 19

TM1637Display display(CLK, DIO);

// ================= TIMER =================

unsigned long stateTimer = 0;
unsigned long blinkTimer = 0;
unsigned long secondTimer = 0;

// ================= BLINK =================

bool blinkState = false;
int blinkCount = 0;

// ================= MODE =================

bool nightMode = false;

// ================= STATE =================

enum TrafficState {
  RED,
  GREEN,
  YELLOW
};

TrafficState state = RED;
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
  pinMode(LED_BLUE, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.showNumberDec(5);

  SetLight(1,0,0);

  stateTimer = millis();
  secondTimer = millis();

  Serial.println("Traffic system started");
}

// ================= LOOP =================

void loop()
{
  // -------- BUTTON --------

  if (!digitalRead(BUTTON_PIN))
  {
    nightMode = !nightMode;
    digitalWrite(LED_BLUE, nightMode);
    delay(300);
  }

  // -------- LDR --------

  int ldr = analogRead(LDR_PIN);
  if (ldr < 1500) nightMode = true;

  // -------- NIGHT MODE --------

  if (nightMode)
  {
    SetLight(0,0,0);

    if (IsReady(blinkTimer, 500))
    {
      blinkState = !blinkState;
      digitalWrite(LED_YELLOW, blinkState);
    }

    display.showNumberDec(0);
    return;
  }

  // -------- COUNTDOWN --------

  if (IsReady(secondTimer, 1000))
  {
    countdown--;
    if (countdown < 0) countdown = 0;
    display.showNumberDec(countdown);
  }

  // -------- TRAFFIC --------

  switch (state)
  {
    case RED:
      SetLight(1,0,0);

      if (IsReady(stateTimer, 5000))
      {
        state = GREEN;
        countdown = 5;
        Serial.println("GREEN");
      }
      break;

    case GREEN:
      SetLight(0,0,1);

      if (IsReady(stateTimer, 5000))
      {
        state = YELLOW;
        countdown = 2;
        blinkCount = 0;
        blinkState = false;
        Serial.println("YELLOW");
      }
      break;

    case YELLOW:

      if (IsReady(blinkTimer, 400))
      {
        blinkState = !blinkState;
        digitalWrite(LED_YELLOW, blinkState);

        if (!blinkState) blinkCount++;

        if (blinkCount >= 3)
        {
          state = RED;
          countdown = 5;
          stateTimer = millis();
          Serial.println("RED");
        }
      }
      break;
  }
}
