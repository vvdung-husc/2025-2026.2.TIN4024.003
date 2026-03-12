#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

/* -------- OLED SH1106 -------- */
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

/* -------- DHT22 -------- */
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/* -------- PIN -------- */
#define LED_PIN D6
#define PIR_PIN D4
#define RELAY1 D8
#define RELAY2 D7

bool ledState = false;

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  dht.begin();

  /* OLED start */
  u8g2.begin();
}

void loop()
{
  /* đọc nhiệt độ và độ ẩm */
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum))
  {
    Serial.println("DHT ERROR");
    return;
  }

  /* đọc PIR */
  int pirState = digitalRead(PIR_PIN);

  /* LED blink */
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);

  /* điều khiển relay */
  if (pirState == HIGH)
  {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
  }
  else
  {
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
  }

  /* Serial Monitor */
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C  ");

  Serial.print("Hum: ");
  Serial.print(hum);
  Serial.print(" %  ");

  Serial.print("Motion: ");
  Serial.println(pirState);

  /* OLED */
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,10,"ESP8266 Controller");

  u8g2.setCursor(0,28);
  u8g2.print("Temp: ");
  u8g2.print(temp);
  u8g2.print(" C");

  u8g2.setCursor(0,42);
  u8g2.print("Hum : ");
  u8g2.print(hum);
  u8g2.print(" %");

  u8g2.setCursor(0,56);

  if(pirState == HIGH)
  {
    u8g2.print("Motion: YES");
  }
  else
  {
    u8g2.print("Motion: NO");
  }

  /* trạng thái LED */
  u8g2.setCursor(80,56);
  if(ledState)
    u8g2.print("LED:ON");
  else
    u8g2.print("LED:OFF");

  u8g2.sendBuffer();

  delay(2000);
}