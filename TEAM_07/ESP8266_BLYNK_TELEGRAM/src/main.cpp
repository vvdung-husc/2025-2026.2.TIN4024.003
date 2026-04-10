/* ===== BLYNK ===== */
#define BLYNK_TEMPLATE_ID "TMPL6pwGzIarE"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "EoArjdBI12gdwkE1dg8ROU_9vmOuNnbE"
/*
  THÔNG TIN NHÓM 7
  1. Trần Hưng Trường Vủ-@truongvu_75
  2. Nguyễn Trọng Quý-@trongquy1811
  3. Trần Quang Tiến-@tientraan
*/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>
#include <TM1637Display.h>

/* ===== WIFI ===== */
char ssid[] = "LAB123";
char pass[] = "55555555";

// TELEGRAM
#define BOTtoken "8339212597:AAFFy_Wh8ayLi_m3vHcv_TZjklBveeJk5iQ"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

/* ===== PIN ===== */
#define LED_PIN D4
#define BTN_PIN D3
#define CLK D6
#define DIO D7
#define DHTPIN D5
#define DHTTYPE DHT11
#define MQ2_PIN A0

/* ===== DEVICE ===== */
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

/* ===== TIMER ===== */
BlynkTimer timer;

/* ===== BIẾN ===== */
bool ledState = true;
unsigned long lastTelegram = 0;
String teamName = "Team 7 - Truong, Quy, Tien";

/* ===== WIFI CONNECT ===== */
void connectWiFi() {
  Serial.print("Connecting WiFi...");
  WiFi.begin(ssid, pass);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry++;

    if (retry > 20) {
      Serial.println("\nRetry WiFi...");
      WiFi.begin(ssid, pass);
      retry = 0;
    }
  }
  Serial.println("\nWiFi connected!");
}

/* ===== BLYNK CONNECT ===== */
void connectBlynk() {
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);

  if (Blynk.connect(5000)) {
    Serial.println("Blynk connected!");
  } else {
    Serial.println("Blynk failed!");
  }
}

/* ===== BUTTON ===== */
void readButton() {
  static int last = HIGH;
  int v = digitalRead(BTN_PIN);

  if (v == last) return;
  last = v;

  if (v == LOW) return;

  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);
  Blynk.virtualWrite(V1, ledState);
}

/* ===== SENSOR ===== */
void sendSensor() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  int gas = analogRead(MQ2_PIN);

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT error");
    return;
  }

  Blynk.virtualWrite(V2, temp);   // nhiệt độ
  Blynk.virtualWrite(V3, hum);    // độ ẩm
  Blynk.virtualWrite(V4, gas);    // gas

  Serial.printf("T: %.1f H: %.1f Gas: %d\n", temp, hum, gas);
}

/* ===== UPTIME ===== */
void sendUptime() {
  int up = millis() / 1000;   // Integer đúng với V0
  Blynk.virtualWrite(V0, up);
  display.showNumberDec(up);
}

/* ===== GỬI TÊN ===== */
void sendName() {
  Blynk.virtualWrite(V5, teamName);
}

/* ===== BLYNK CONTROL ===== */
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState ? LOW : HIGH);
}

/* ===== TELEGRAM ===== */
void handleTelegram(int n) {
  for (int i = 0; i < n; i++) {

    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start") {
      bot.sendMessage(chat_id,
        "/led_on\n/led_off\n/status\n/get_weather", "");
    }

    if (text == "/led_on") {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED ON", "");
    }

    if (text == "/led_off") {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED OFF", "");
    }

    if (text == "/status") {
      String st = digitalRead(LED_PIN) == LOW ? "ON" : "OFF";
      bot.sendMessage(chat_id, "LED: " + st, "");
    }

    if (text == "/get_weather") {
      float t = dht.readTemperature();
      float h = dht.readHumidity();

      String msg = "Temp: " + String(t) + " C\n";
      msg += "Hum: " + String(h) + " %";

      bot.sendMessage(chat_id, msg, "");
    }
  }
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, HIGH);

  WiFi.begin(ssid, password);

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  client.setInsecure();

  connectBlynk();

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, sendUptime);
  timer.setInterval(5000L, sendName);
}

/* ===== LOOP ===== */
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!Blynk.connected()) {
    connectBlynk();
  }

  Blynk.run();
  timer.run();
  readButton();

  if (millis() - lastTelegram > 2000) {
    int n = bot.getUpdates(bot.last_message_received + 1);

    while (n) {
      handleTelegram(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTelegram = millis();
  }
}