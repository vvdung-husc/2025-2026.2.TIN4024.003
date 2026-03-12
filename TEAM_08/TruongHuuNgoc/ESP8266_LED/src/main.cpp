#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// ===== PIN CONFIG =====
#define DHTPIN 0
#define DHTTYPE DHT11
#define LED_PIN 2
#define BTN_PIN 12

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);

// OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== BIẾN HỆ THỐNG =====
unsigned long currentMiliseconds = 0;
unsigned long uptimeSeconds = 0;

// bool counterActive = true;

// ===== HÀM CHỐNG DỘI =====
bool IsReady(unsigned long &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;

  ulTimer = currentMiliseconds;
  return true;
}

// ===== XỬ LÝ NÚT =====
// void updateButton()
// {
//   static unsigned long lastTime = 0;
//   static int lastValue = HIGH;

//   if (!IsReady(lastTime, 50))
//     return;

//   int v = digitalRead(BTN_PIN);

//   if (v == lastValue)
//     return;

//   lastValue = v;

//   if (v == LOW)
//     return;

//   counterActive = !counterActive;

//   if (counterActive)
//     Serial.println("SYSTEM ON");
//   else
//     Serial.println("SYSTEM OFF");
// }

// ===== LED NHẤP NHÁY =====
void blinkLED()
{
  static unsigned long lastTime = 0;
  static bool ledState = false;

  // if (!counterActive)
  // {
  //   digitalWrite(LED_PIN, LOW);
  //   return;
  // }

  if (!IsReady(lastTime, 1000))
    return;

  ledState = !ledState;

  digitalWrite(LED_PIN, ledState);
}

// ===== ĐỌC CẢM BIẾN =====
void readSensor()
{
  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 2000))
    return;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("DHT ERROR");
    return;
  }

  Serial.print("Temp: ");
  Serial.println(t);

  Serial.print("Hum: ");
  Serial.println(h);

  // HIỂN THỊ OLED
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(0, 15, "ESP8266 SENSOR");

  char tempStr[20];
  sprintf(tempStr, "Temp: %.1f C", t);
  u8g2.drawStr(0, 35, tempStr);

  char humStr[20];
  sprintf(humStr, "Hum : %.1f %%", h);
  u8g2.drawStr(0, 55, humStr);

  u8g2.sendBuffer();
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();
  u8g2.begin();

  Serial.println("SYSTEM READY");
}

// ===== LOOP =====
void loop()
{
  currentMiliseconds = millis();

  // updateButton();
  blinkLED();
  readSensor();
}