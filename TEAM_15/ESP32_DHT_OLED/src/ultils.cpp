/*
Thông tin nhóm 15:
1. Lê Tăng Phước
2. Đặng Hữu Trung
*/

#include "ultils.h"

void showOLED(float temp, float hum) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.setCursor(0, 16);
  display.print("Humidity: ");
  display.print(hum);
  display.println(" %");

  display.setCursor(0, 32);

  if (temp < 13) display.println("Status: TOO COLD");
  else if (temp < 20) display.println("Status: COLD");
  else if (temp < 25) display.println("Status: COOL");
  else if (temp < 30) display.println("Status: WARM");
  else if (temp < 35) display.println("Status: HOT");
  else display.println("Status: TOO HOT");

  display.display();
}

void updateLED(float temp) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_CYAN, LOW);

  if (temp < 20) {
    digitalWrite(LED_CYAN, HIGH);
  }
  else if (temp < 30) {
    digitalWrite(LED_YELLOW, HIGH);
  }
  else {
    digitalWrite(LED_RED, HIGH);
  }
}
