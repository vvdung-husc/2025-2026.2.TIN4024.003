#include <Arduino.h>

#define LED_RED 17
#define LED_YELLOW 16
#define LED_GREEN 18

unsigned long timer = 0;
bool ledState = false;

unsigned long blinkTimer = 0;
bool blinkState = false;
int blinkCount = 0;


enum TrafficState
{
  RED,
  GREEN,
  YELLOW
};

TrafficState state = RED;

// non blocking timer
bool IsReady(unsigned long &t, uint32_t ms)
{
  if (millis() - t < ms)
    return false;
  t = millis();
  return true;
}

void SetLight(bool r, bool y, bool g)
{
  digitalWrite(LED_RED, r);
  digitalWrite(LED_YELLOW, y);
  digitalWrite(LED_GREEN, g);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  SetLight(1, 0, 0); // start RED
  Serial.println("Traffic light started");
}

void loop()
{
  switch (state)
  {
  case RED:
    if (IsReady(timer, 5000))
    { 
      SetLight(0, 0, 1);
      state = GREEN;
      Serial.println("GREEN");
    }
    if(IsReady(blinkTimer, 500))
    {
      blinkState = !blinkState;
      digitalWrite(LED_RED, blinkState ? HIGH : LOW);
      if(blinkState == false)
      {
        blinkCount++;
      }
      if(blinkCount >= 3)
      {
        blinkCount = 0;
        // reset to solid red
        digitalWrite(LED_RED, HIGH);
      }
    }
    break;

  case GREEN:
    if (IsReady(timer, 5000))
    {
      SetLight(0, 1, 0);
      state = YELLOW;
      Serial.println("YELLOW");
    }
    if(IsReady(blinkTimer, 500))
    {
      blinkState = !blinkState;
      digitalWrite(LED_GREEN, blinkState ? HIGH : LOW);
      if(blinkState == false)
      {
        blinkCount++;
      }
      if(blinkCount >= 3)
      {
        blinkCount = 0;
        // reset to solid green
        digitalWrite(LED_GREEN, HIGH);
      }
    }
    break;

  case YELLOW:

  if (blinkCount == 0)
  {
    // reset khi mới vào vàng
    blinkState = false;
    blinkTimer = millis();
  }

  if (IsReady(blinkTimer, 500))
  {
    blinkState = !blinkState;
    digitalWrite(LED_YELLOW, blinkState ? HIGH : LOW);

    if (blinkState == false)
    {
      blinkCount++;
    }

    if (blinkCount >= 3)
    {
      blinkCount = 0;
      blinkState = false;

      SetLight(1, 0, 0);   // sang đỏ
      timer = millis();
      state = RED;

      Serial.println("RED");
    }
  }

  break;

  }
}
