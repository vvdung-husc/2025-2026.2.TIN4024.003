
#include <esp32-hal-gpio.h>
#define LED_RED 25
#define LED_YELLOW 26
#define LED_GREEN 27

void allOff()
{
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  allOff(); 
}

void loop()
{
  // ĐỎ: 2 giây
  allOff();
  digitalWrite(LED_RED, HIGH);
  delay(2000);

  // VÀNG: 3 giây
  allOff();
  digitalWrite(LED_YELLOW, HIGH);
  delay(3000);

  // XANH: 5 giây
  allOff();
  digitalWrite(LED_GREEN, HIGH);
  delay(5000);
}
