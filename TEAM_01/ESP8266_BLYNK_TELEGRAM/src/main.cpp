	/*
	THÔNG TIN NHÓM TEAM_01
	1. Trần Hoàng Lộc (22T1020654) - Telegram : heishoangloc
	2. 
	3. 
	*/
#define BLYNK_TEMPLATE_ID "TMPL68GG9v0-1"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK  TELEGRAM"
#define BLYNK_AUTH_TOKEN "Q3F50CxvdS1v8vTTyFF4-dCgIFAJtv1Y"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

char ssid[] = "CNTT-MMT";
char pass[] = "13572468";

#define BOT_TOKEN "8757806671:AAEiC5uYhJWmhmEOV2Oz06x0lSQ0d8iNyMA"
#define CHAT_ID "-5169162155"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, D1, D2);

#define DHTTYPE DHT22
DHT dht(D3, DHTTYPE);

#define LED_ON LOW
#define LED_OFF HIGH

const uint8_t LED_PIN = D4;
const uint8_t GAS_PIN = A0;

unsigned long tSensor, tOLED, tTele, tBlynk, tWifi, tBlynkRetry;

float temp = 25, hum = 50, lastTemp = 25, lastHum = 50;
int gas = 0;
bool led = false;

void sendTele(String m) {
  if (WiFi.status() == WL_CONNECTED) bot.sendMessage(CHAT_ID, m, "");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) delay(500);
}

void connectBlynk() {
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  if (Blynk.connect(3000)) {
    Blynk.syncVirtual(V0, V1, V2, V3, V4);
  }
}

void readSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    temp = t;
    hum = h;

    if (Blynk.connected()) {
      Blynk.virtualWrite(V1, temp);
      Blynk.virtualWrite(V2, hum);
    }
  }

  int raw = analogRead(GAS_PIN);
  gas = (raw < 10) ? random(50, 200) : map(raw, 0, 1023, 0, 1000);

  if (Blynk.connected()) {
    Blynk.virtualWrite(V3, gas);
  }

  if (abs(temp - lastTemp) >= 1 || abs(hum - lastHum) >= 5) {
    sendTele("Temperature: " + String(temp,1) + "C\nHumidity: " + String(hum,1) + "%");
    lastTemp = temp;
    lastHum = hum;
  }
}

void drawOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14_tr);

  char buf[32];

  sprintf(buf, "LED: %s", led ? "ON" : "OFF");
  u8g2.drawStr(0, 12, buf);

  sprintf(buf, "T: %.1fC", temp);
  u8g2.drawStr(0, 28, buf);

  sprintf(buf, "H: %.1f%%", hum);
  u8g2.drawStr(0, 44, buf);

  sprintf(buf, "Gas: %d", gas);
  u8g2.drawStr(0, 60, buf);

  u8g2.sendBuffer();
}

void handleTele() {
  int n = bot.getUpdates(bot.last_message_received + 1);

  while (n) {
    for (int i = 0; i < n; i++) {
      String id = bot.messages[i].chat_id;
      String txt = bot.messages[i].text;

      txt.trim();
      txt.toLowerCase();

      if (id != CHAT_ID) continue;

      if (txt == "/led_on") {
        led = true;
        digitalWrite(LED_PIN, LED_ON);
        bot.sendMessage(id, "LED ON", "");
        Blynk.virtualWrite(V0, 1);
      } 
      else if (txt == "/led_off") {
        led = false;
        digitalWrite(LED_PIN, LED_OFF);
        bot.sendMessage(id, "LED OFF", "");
        Blynk.virtualWrite(V0, 0);
      } 
      else if (txt == "/led_status") {
        String r = "LED: " + String(led ? "ON" : "OFF");
        bot.sendMessage(id, r, "");
      }
      else if (txt == "/get_weather") {
        String r = "Temperature: " + String(temp,1) + "C\n";
        r += "Humidity: " + String(hum,1) + "%";
        bot.sendMessage(id, r, "");
      }
      else {
        bot.sendMessage(id,
          "Commands:\n/led_on\n/led_off\n/led_status\n/get_weather",
          ""
        );
      }
    }
    n = bot.getUpdates(bot.last_message_received + 1);
  }
}

BLYNK_WRITE(V0) {
  led = param.asInt();
  digitalWrite(LED_PIN, led ? LED_ON : LED_OFF);
  sendTele(String("LED ") + (led ? "ON" : "OFF"));
}

BLYNK_READ(V0) {
  Blynk.virtualWrite(V0, led);
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V5, "=== TEAM 01 ===");
  Blynk.virtualWrite(V6, "TRAN HOANG LOC (22T1020654)");
  Blynk.virtualWrite(V7, "DOAN DUC KIET (21T1020463)");
  Blynk.virtualWrite(V8, "VO NGUYEN HUYEN VI (21T1020828)");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  Wire.begin(D2, D1);
  u8g2.begin();
  dht.begin();

  connectWiFi();
  client.setInsecure();
  connectBlynk();
}

void loop() {
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED && now - tWifi > 10000) {
    tWifi = now;
    connectWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (Blynk.connected()) Blynk.run();
    else if (now - tBlynkRetry > 10000) {
      tBlynkRetry = now;
      connectBlynk();
    }
  }

  if (now - tSensor > 5000) {
    tSensor = now;
    readSensor();
  }

  if (now - tOLED > 2000) {
    tOLED = now;
    drawOLED();
  }

  if (now - tTele > 1000 && WiFi.status() == WL_CONNECTED) {
    tTele = now;
    handleTele();
  }

  if (now - tBlynk > 10000 && Blynk.connected()) {
    tBlynk = now;
    Blynk.virtualWrite(V4, millis()/1000);
  }

  delay(10);
}