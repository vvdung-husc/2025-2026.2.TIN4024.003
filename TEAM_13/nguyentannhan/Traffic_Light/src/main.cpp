#include <Arduino.h>
#include <TM1637Display.h>

// --- CẤU HÌNH CHÂN ---
#define CLK 18 
#define DIO 19

#define RED_PIN     27
#define YELLOW_PIN  26
#define GREEN_PIN   25
#define STATUS_LED  21   // led1 – sáng khi DỪNG

#define BUTTON_PIN  23

TM1637Display display(CLK, DIO);

// Trạng thái hệ thống
bool isRunning = false;

void setup() {
  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(0x0f);

  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setSegments(data);

  // Ban đầu: DỪNG → led1 sáng
  digitalWrite(STATUS_LED, HIGH);
}

// --- KIỂM TRA NÚT ---
void checkButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      isRunning = !isRunning;

      if (isRunning) {
        Serial.println("System STARTED");
        digitalWrite(STATUS_LED, LOW);   // Chạy → tắt led1
      } else {
        Serial.println("System PAUSED");
        digitalWrite(STATUS_LED, HIGH);  // Dừng → bật led1
      }

      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

// --- SMART DELAY ---
void smartDelay(int seconds) {
  for (int i = seconds; i > 0; i--) {
    display.showNumberDec(i);

    unsigned long startMillis = millis();
    while (millis() - startMillis < 1000) {
      checkButton();

      // Nếu đang DỪNG
      while (!isRunning) {
        digitalWrite(STATUS_LED, HIGH); // luôn sáng khi dừng
        checkButton();
      }
    }
  }
}

void loop() {
  // Nếu chưa chạy
  if (!isRunning) {
    checkButton();

    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    digitalWrite(STATUS_LED, HIGH); // dừng → led1 sáng
    return;
  }

  // --- ĐÈN ĐỎ ---
  digitalWrite(STATUS_LED, LOW);
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  smartDelay(10);

  // --- ĐÈN XANH ---
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  smartDelay(7);

  // --- ĐÈN VÀNG ---
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  smartDelay(3);
}
