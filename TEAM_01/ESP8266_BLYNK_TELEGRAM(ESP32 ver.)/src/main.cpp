#define BLYNK_TEMPLATE_ID "TMPL68GG9v0-1"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK  TELEGRAM"
#define BLYNK_AUTH_TOKEN "Q3F50CxvdS1v8vTTyFF4-dCgIFAJtv1Y"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi (Wokwi)
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// Telegram
#define BOT_TOKEN "8757806671:AAEiC5uYhJWmhmEOV2Oz06x0lSQ0d8iNyMA"
#define CHAT_ID "-5169162155"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// -------- DHT22 --------
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// -------- LED --------
#define LED_PIN 5
bool ledState = false;

// -------- MQ2 --------
#define MQ2_PIN 34

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- DATA --------
float temp, hum;
int gas;

// -------- TIME --------
unsigned long startMillis;

// -------- TIMER --------
BlynkTimer timer;

// -------- BLYNK CONTROL --------
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// -------- READ SENSOR --------
void readSensor() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    temp = random(25, 35);
    hum = random(50, 80);
  }

  gas = analogRead(MQ2_PIN);
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(V5,
    "=== TEAM 01 ==="
  );
  Blynk.virtualWrite(V6,
    "TRẦN HOÀNG LỘC (22T1020654)"
  );
  Blynk.virtualWrite(V7,
    "ĐOÀN ĐỨC KIỆT (21T1020463)"
  );
  Blynk.virtualWrite(V8,
    "VÕ NGUYỄN HUYỀN VI (21T1020828)"
  );
  
}

// -------- OLED --------
void displayOLED() {
  display.clearDisplay();

  display.setCursor(0,0);
  display.println("ESP32 IoT");

  display.print("Temp: "); display.println(temp);
  display.print("Hum : "); display.println(hum);
  display.print("Gas : "); display.println(gas);

  display.print("LED : ");
  display.println(ledState ? "ON" : "OFF");

  unsigned long uptime = (millis() - startMillis)/1000;
  display.print("Time: "); display.println(uptime);

  display.display();
}

// -------- BLYNK --------
void sendBlynk() {
  unsigned long uptime = (millis() - startMillis)/1000;

  Blynk.virtualWrite(V1, temp);
  Blynk.virtualWrite(V2, hum);
  Blynk.virtualWrite(V3, gas);
  Blynk.virtualWrite(V4, uptime);
}

// -------- TELEGRAM COMMAND --------
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      String text = bot.messages[i].text;

      if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        ledState = true;
        bot.sendMessage(CHAT_ID, "LED ON", "");
      }

      else if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
        bot.sendMessage(CHAT_ID, "LED OFF", "");
      }

      else if (text == "/led_status") {
        bot.sendMessage(CHAT_ID, ledState ? "LED ON" : "LED OFF", "");
      }

      else if (text == "/get_weather") {
        String msg = "Temp: " + String(temp) + "\nHum: " + String(hum);
        bot.sendMessage(CHAT_ID, msg, "");
      }
    }

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

// -------- TELEGRAM NOTIFY --------
float lastTemp = 0;
float lastHum = 0;

void notifyTelegram() {
  if (abs(temp - lastTemp) > 1 || abs(hum - lastHum) > 2) {
    String msg = "Update:\nTemp: " + String(temp) + "\nHum: " + String(hum);
    bot.sendMessage(CHAT_ID, msg, "");

    lastTemp = temp;
    lastHum = hum;
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  display.setTextSize(1);

  WiFi.begin(ssid, pass);

  client.setInsecure(); // VERY IMPORTANT

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  startMillis = millis();

  timer.setInterval(2000L, readSensor);
  timer.setInterval(2000L, displayOLED);
  timer.setInterval(3000L, sendBlynk);
  timer.setInterval(5000L, notifyTelegram);
}

// -------- LOOP --------
void loop() {
  Blynk.run();
  timer.run();
  handleTelegram();
}