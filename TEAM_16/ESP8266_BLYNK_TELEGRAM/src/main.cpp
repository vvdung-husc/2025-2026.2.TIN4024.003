/*
THÔNG TIN NHÓM 16
1. Lê Anh Quân - Telegram: @tomle2111
2. Phạm Ngọc Khanh - Telegram: @thang3008
*/
#define BLYNK_TEMPLATE_ID "TMPL6KMb2ddys"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "9AsokM_2bp8KauSGqA2zB8imt8ccbo6A"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>

// ==== WIFI ====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
// ==== TELEGRAM ====
#define BOT_TOKEN "8631035811:AAGdad4I1gMrUliK4C6dbGYEBz4qIZPgtRs"
WiFiClient client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ==== DHT ====
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ==== OLED ====
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ==== LED ====
#define LED_PIN 2
bool ledState = false;

// ==== TIMER ====
BlynkTimer timer;

// ==== DATA ====
float temp, hum;
int gasValue;

// =====================================
// OLED
// =====================================
void updateOLED() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.print("Temp: ");
  display.println(temp);

  display.print("Humi: ");
  display.println(hum);

  display.print("Gas: ");
  display.println(gasValue);

  display.display();
}

// =====================================
// BLYNK LED CONTROL
// =====================================
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

// =====================================
// Gửi dữ liệu lên Blynk
// =====================================
void sendData() {
  temp = dht.readTemperature();
  hum  = dht.readHumidity();

  gasValue = random(200, 800); // fake MQ2

  Blynk.virtualWrite(V0, millis()/1000); // uptime
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, gasValue);
  Blynk.virtualWrite(V5, "TEAM 16");

  updateOLED();
}

// =====================================
// TELEGRAM COMMAND
// =====================================
void handleTelegram() {
  int n = bot.getUpdates(bot.last_message_received + 1);

  while (n) {
    for (int i = 0; i < n; i++) {
      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;

      if (text == "/led_on") {
        digitalWrite(LED_PIN, HIGH);
        bot.sendMessage(chat_id, "LED ON", "");
      }
      else if (text == "/led_off") {
        digitalWrite(LED_PIN, LOW);
        bot.sendMessage(chat_id, "LED OFF", "");
      }
      else if (text == "/led_status") {
        bot.sendMessage(chat_id, ledState ? "ON" : "OFF", "");
      }
      else if (text == "/get_weather") {
        String msg = "Temp: " + String(temp) + "\nHumi: " + String(hum);
        bot.sendMessage(chat_id, msg, "");
      }
    }
    n = bot.getUpdates(bot.last_message_received + 1);
  }
}

// =====================================
// SETUP
// =====================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  dht.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendData);
}

// =====================================
// LOOP
// =====================================
void loop() {
  Blynk.run();
  timer.run();
  handleTelegram();
}