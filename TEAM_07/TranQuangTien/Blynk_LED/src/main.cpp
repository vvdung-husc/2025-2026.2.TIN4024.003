#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* Blynk */
#define BLYNK_TEMPLATE_ID "TMPL6w14opWnl"
#define BLYNK_TEMPLATE_NAME "Blynk LED"
#define BLYNK_AUTH_TOKEN "36Sk7d945Qqz7ziVK0t_9YFp9swectYQ"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

/* WiFi Wokwi */
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* Pin */
#define btnBLED 23
#define pinBLED 21

#define CLK 18
#define DIO 19

#define DHTPIN 16
#define DHTTYPE DHT22

/* Device */
TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);

/* Blynk timer */
BlynkTimer timer;

/* Biến */
bool blueButtonON = true;

/* ===== Đọc nút ===== */
void updateBlueButton() {

  static int lastValue = HIGH;

  int v = digitalRead(btnBLED);

  if (v == lastValue) return;

  lastValue = v;

  if (v == LOW) return;

  blueButtonON = !blueButtonON;

  digitalWrite(pinBLED, blueButtonON);

  Blynk.virtualWrite(V1, blueButtonON);

  if (!blueButtonON) {
    display.clear();
  }

}

/* ===== Gửi dữ liệu Blynk ===== */
void sendDataBlynk() {

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT read error");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C  Hum: ");
  Serial.println(hum);

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);

}

/* ===== Uptime ===== */
void sendUptime() {

  unsigned long uptime = millis() / 1000;

  Blynk.virtualWrite(V0, uptime);

  if (blueButtonON) {
    display.showNumberDec(uptime);
  }

}

/* ===== Nhận từ Blynk ===== */
BLYNK_WRITE(V1) {

  blueButtonON = param.asInt();

  digitalWrite(pinBLED, blueButtonON);

  if (!blueButtonON) {
    display.clear();
  }

}

/* ===== Setup ===== */
void setup() {

  Serial.begin(115200);

  pinMode(pinBLED, OUTPUT);
  pinMode(btnBLED, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();

  Serial.println("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("WiFi connected");

  digitalWrite(pinBLED, blueButtonON);

  Blynk.virtualWrite(V1, blueButtonON);

  /* Timer */
  timer.setInterval(2000L, sendDataBlynk);
  timer.setInterval(1000L, sendUptime);

}

/* ===== Loop ===== */
void loop() {

  Blynk.run();
  timer.run();

  updateBlueButton();

}