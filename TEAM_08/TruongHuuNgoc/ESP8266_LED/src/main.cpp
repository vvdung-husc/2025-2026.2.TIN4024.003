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

DHT dht(DHTPIN, DHTTYPE);

// OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

unsigned long currentMiliseconds = 0;

bool IsReady(unsigned long &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;

  ulTimer = currentMiliseconds;
  return true;
}

// ===== LED =====
void blinkLED()
{
  static unsigned long lastTime = 0;
  static bool ledState = false;

  if (!IsReady(lastTime, 1000))
    return;

  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);
}

// ===== SENSOR =====
void readSensor()
{
  static unsigned long lastTime = 0;

  if (!IsReady(lastTime, 2000))
    return;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.drawStr(0, 15, "ESP8266 SENSOR");

  if (isnan(h) || isnan(t))
  {
    Serial.println("DHT ERROR");
    u8g2.drawStr(0, 40, "Sensor Error");
  }
  else
  {
    Serial.print("Temp: ");
    Serial.println(t);

    Serial.print("Hum: ");
    Serial.println(h);

    char tempStr[20];
    sprintf(tempStr, "Temp: %.1f C", t);
    u8g2.drawStr(0, 35, tempStr);

    char humStr[20];
    sprintf(humStr, "Hum : %.1f %%", h);
    u8g2.drawStr(0, 55, humStr);
  }

  u8g2.sendBuffer();
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();
  delay(2000);

  u8g2.begin();

  Serial.println("SYSTEM READY");
}

// ===== LOOP =====
void loop()
{
  currentMiliseconds = millis();

  blinkLED();
  readSensor();
}