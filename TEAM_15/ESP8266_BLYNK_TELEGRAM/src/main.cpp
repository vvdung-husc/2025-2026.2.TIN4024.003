/* THÔNG TIN NHÓM X
1. Lê Tăng Phước - Telegram: @Phuocle2511
2. 
*/

#define BLYNK_TEMPLATE_ID "TMPL6NoZw2XyB"
#define BLYNK_TEMPLATE_NAME "ESP8266"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//================ WIFI + BLYNK =================
char ssid[] = "CNTT-MMT";
char pass[] = "13572468";
char auth[] = "yDuouDkO3GWSjcEfwQgIlHsdV5vZMKOO";

//================ TELEGRAM =================
#define BOT_TOKEN "8784704551:AAFIRQJhtHih75D6MjfGPtiCDG69A6NtyJY"
#define CHAT_ID "-5149121218"
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

//================ HARDWARE =================
#define LED_BUILTIN 2
#define DHTPIN 0
#define DHTTYPE DHT22
#define OLED_SDA 4
#define OLED_SCL 5

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
BlynkTimer timer;

//================ BIẾN =================
float temperature = 0;
float humidity = 0;
int gas = 0;
bool ledState = false;

unsigned long lastTelegramCheck = 0;
unsigned long lastTempTelegram = 0;
const unsigned long telegramCooldown = 60000; // 1 phút

//================ BLYNK =================
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
}

//================ TELEGRAM =================
void handleTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    if (text == "/led_on") {
      digitalWrite(LED_BUILTIN, LOW);
      ledState = true;
      bot.sendMessage(CHAT_ID, "LED ON", "");
    }
    else if (text == "/led_off") {
      digitalWrite(LED_BUILTIN, HIGH);
      ledState = false;
      bot.sendMessage(CHAT_ID, "LED OFF", "");
    }
    else if (text == "/led_status") {
      bot.sendMessage(CHAT_ID, ledState ? "LED ON" : "LED OFF", "");
    }
    else if (text == "/get_weather") {
      String msg = "Nhiet do: " + String(temperature) + "C\n";
      msg += "Do am: " + String(humidity) + "%";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

//================ ĐỌC SENSOR =================
void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
    gas = random(200, 800); // giả lập MQ2

    // gửi lên Blynk
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);
    Blynk.virtualWrite(V3, gas);
    long uptime = millis() / 1000;
    Blynk.virtualWrite(V4, uptime);

    // Telegram khi nhiệt độ thay đổi >1C và cooldown 1 phút
    static float lastTemp = 0;
    if (abs(temperature - lastTemp) > 1 && (millis() - lastTempTelegram > telegramCooldown)) {
      String msg = "Nhiet do thay doi!\n";
      msg += String(temperature) + "C";
      bot.sendMessage(CHAT_ID, msg, "");
      lastTemp = temperature;
      lastTempTelegram = millis();
    }
  }
}

//================ OLED =================
void displayOLED() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 12, ("Temp: " + String(temperature) + "C").c_str());
  oled.drawUTF8(0, 26, ("Humi: " + String(humidity) + "%").c_str());
  oled.drawUTF8(0, 40, ("Gas : " + String(gas)).c_str());
  long uptime = millis() / 1000;
  oled.drawUTF8(0, 54, ("Up: " + String(uptime) + "s").c_str());
  oled.sendBuffer();
}

//================ SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  dht.begin();
  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();

  WiFi.begin(ssid, pass);
  client.setInsecure();
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  Blynk.config(auth);
  Blynk.connect();

  // Timer
  timer.setInterval(2000L, readSensor);
  timer.setInterval(1000L, displayOLED);
}

//================ LOOP =================
void loop() {
  // WiFi reconnect nếu mất kết nối
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Reconnecting...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    Serial.println("WiFi Reconnected");
    Blynk.connect();
  }

  Blynk.run();
  timer.run();

  // Telegram check mỗi 2s
  if (millis() - lastTelegramCheck > 2000) {
    handleTelegram();
    lastTelegramCheck = millis();
  }
}