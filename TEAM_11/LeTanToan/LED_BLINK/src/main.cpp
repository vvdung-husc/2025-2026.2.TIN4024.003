#include <Arduino.h>

// Định nghĩa các chân GPIO
const int PIN_RED    = 13;
const int PIN_YELLOW = 12;
const int PIN_GREEN  = 14;

// Thời gian delay (ms)
const int TIME_RED    = 3000;
const int TIME_YELLOW = 3000;
const int TIME_GREEN  = 3000;

// Hàm tiện ích để bật đèn này, tắt đèn kia
void setTrafficLight(bool r, bool y, bool g) {
    digitalWrite(PIN_RED,    r ? HIGH : LOW);
    digitalWrite(PIN_YELLOW, y ? HIGH : LOW);
    digitalWrite(PIN_GREEN,  g ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);
    
    // Set mode output cho các chân
    pinMode(PIN_RED,    OUTPUT);
    pinMode(PIN_YELLOW, OUTPUT);
    pinMode(PIN_GREEN,  OUTPUT);

    Serial.println("Traffic Light System Started");
}

void loop() {
    // 1. Đèn XANH sáng
    Serial.println("GO (Green)");
    setTrafficLight(false, false, true);
    delay(TIME_GREEN);

    // 2. Đèn VÀNG sáng
    Serial.println("SLOW (Yellow)");
    setTrafficLight(false, true, false);
    delay(TIME_YELLOW);

    // 3. Đèn ĐỎ sáng
    Serial.println("STOP (Red)");
    setTrafficLight(true, false, false);
    delay(TIME_RED);
}