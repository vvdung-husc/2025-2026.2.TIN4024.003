/*
Thông tin nhóm 15:
1. Lê Tăng Phước
2. Đặng Hữu Trung
3. Trần Nguyễn Hoàng Quân
*/

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <Wire.h>

#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ===== PIN THEO SO DO ===== */
#define DHTPIN     16
#define DHTTYPE    DHT22

#define LED_RED     4
#define LED_YELLOW  2
#define LED_CYAN    15

/* ===== OLED ===== */
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

extern DHT dht;
extern Adafruit_SSD1306 display;

void setupPins();

#endif
