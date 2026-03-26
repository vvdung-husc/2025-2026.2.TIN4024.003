#include <Arduino.h>
#include "utils.h"

#include <Wire.h>
#include <U8g2lib.h>

// SỬA LẠI CHÂN CHO KHỚP VỚI BO MẠCH CỦA BẠN
#define gPIN 15 // Chân D8 (Relay 1) - Thay cho đèn Xanh
#define yPIN 13 // Chân D7 (Relay 2) - Thay cho đèn Vàng
#define rPIN 2  // Chân D4 (LED tích hợp trên chip) - Thay cho đèn Đỏ

// SỬA LẠI CHÂN OLED THEO SƠ ĐỒ MẠCH
#define OLED_SDA 4 // Chân D2
#define OLED_SCL 5 // Chân D1

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool WelcomeDisplayTimeout(uint msSleep = 5000){
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

void setup() {
  Serial.begin(115200);
  pinMode(gPIN, OUTPUT);
  pinMode(yPIN, OUTPUT);
  pinMode(rPIN, OUTPUT);
  
  digitalWrite(gPIN, LOW);
  digitalWrite(yPIN, LOW);
  digitalWrite(rPIN, HIGH); // LED tích hợp trên ESP8266 dùng mức HIGH để tắt

  // Khởi tạo I2C với chân SDA và SCL chuẩn của mạch
  Wire.begin(OLED_SDA, OLED_SCL);  

  oled.begin();
  oled.clearBuffer();
  
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");  
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "Lập trình IoT"); 

  oled.sendBuffer();
}

void updateTemperature(){
  static ulong lastTimer = 0;  
  if (!IsReady(lastTimer, 2000)) return;

  int t = random(-40, 81); // Sinh nhiệt độ ngẫu nhiên từ -40°C đến 80°C

  char buffer[20];
  sprintf(buffer, "Nhiệt độ: %d°C", t);

  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 42, buffer);
  oled.sendBuffer();

  Serial.println(buffer);
}

void loop() {
  if (!WelcomeDisplayTimeout()) return;
  updateTemperature();
}