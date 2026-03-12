#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

/* ----------- OLED SH1106 ----------- */
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

/* ----------- DHT ----------- */
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/* ----------- PIN ----------- */
#define LED_PIN LED_BUILTIN
#define PIR_PIN D4
#define RELAY1 D8
#define RELAY2 D7

void setup() {

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

void loop() {

  /* LED blink */
  digitalWrite(LED_PIN, LOW);
  delay(200);
  digitalWrite(LED_PIN, HIGH);
  delay(200);

  /* đọc nhiệt độ và độ ẩm */
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  /* kiểm tra lỗi cảm biến */
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  /* in ra Serial */
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  /* đọc PIR */
  int pirState = digitalRead(PIR_PIN);

  /* hiển thị OLED */
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,10,"ESP8266 Controller");

  u8g2.setCursor(0,28);
  u8g2.print("Temp: ");
  u8g2.print(temp);
  u8g2.print(" C");

  u8g2.setCursor(0,44);
  u8g2.print("Hum : ");
  u8g2.print(hum);
  u8g2.print(" %");

  u8g2.setCursor(0,60);

  if(pirState == HIGH)
  {
    u8g2.print("Motion: YES");
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
  }
  else
  {
    u8g2.print("Motion: NO");
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
  }

  u8g2.sendBuffer();

  delay(2000);
}