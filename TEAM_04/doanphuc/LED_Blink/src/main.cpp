#include <Arduino.h>         // Sửa lỗi pinMode, Serial
#include <TM1637Display.h>

// Khai báo các chân kết nối
#define LED_RED 25
#define LED_YELLOW 33
#define LED_GREEN 32
#define PIN_CLK 15
#define PIN_DIO 2

const int RED_TIME = 15;
const int GREEN_TIME = 10;
const int YELLOW_TIME = 3;

TM1637Display display(PIN_CLK, PIN_DIO);

// --- DI CHUYỂN CÁC HÀM NÀY LÊN TRÊN ĐỂ HẾT LỖI "NOT DECLARED" ---

void allLedsOff() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

void runTrafficLight(int ledPin, int seconds) {
  allLedsOff();
  digitalWrite(ledPin, HIGH);
  for (int i = seconds; i >= 0; i--) {
    display.showNumberDec(i);
    delay(1000);
  }
}

// --- CÁC HÀM CHÍNH ---

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  display.setBrightness(0x0f);
  Serial.begin(115200);
}

void loop() {
  runTrafficLight(LED_RED, RED_TIME);     // 1. Đỏ
  runTrafficLight(LED_GREEN, GREEN_TIME); // 2. Xanh
  runTrafficLight(LED_YELLOW, YELLOW_TIME); // 3. Vàng
}