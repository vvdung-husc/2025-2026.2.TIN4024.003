#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 17
#define OLED_SCL 16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int leds[] = {21, 22, 23}; // đỏ, xanh, vàng
const char* names[] = {"RED", "BLUE", "YELLOW"};

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) {
    pinMode(leds[i], OUTPUT);
  }

  // Khởi động I2C cho OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // Khởi động OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(" OLED not found");
    while (1);
  }

  display.clearDisplay();
  display.display();
}

void showCountdown(int sec, const char* label) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(label);

  display.setTextSize(4);
  display.setCursor(50, 25);
  display.println(sec);

  display.display();
}

void loop() {
  for (int i = 0; i < 3; i++) {

    // Tắt hết đèn
    for (int j = 0; j < 3; j++) {
      digitalWrite(leds[j], LOW);
    }

    // Bật 1 đèn
    digitalWrite(leds[i], HIGH);

    // Đếm ngược 3 → 1
    for (int s = 3; s >= 1; s--) {
      showCountdown(s, names[i]);
      delay(1000);
    }
  }
}
