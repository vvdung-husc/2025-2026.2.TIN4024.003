#define BLYNK_TEMPLATE_ID "TMPLxxxx"
#define BLYNK_TEMPLATE_NAME "ESP32 LED TM1637"
#define BLYNK_AUTH_TOKEN "xZQsIpgV3kFi-eNKaTtM3s8z45uaLgAS"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi
char ssid[] = "15/5 NGUYEN DO CUNG";
char pass[] = "hochiminh0209";

// DHT22
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// TM1637
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// LED
#define LED_PIN 21

// BUTTON
#define BUTTON_PIN 23

BlynkTimer timer;

int uptime = 0;
bool ledState = false;

//////////////////////////////////////////////////

// Đọc nhiệt độ độ ẩm
void readSensor() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read DHT!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("  Humidity: ");
  Serial.println(h);

  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
}

//////////////////////////////////////////////////

// Đếm thời gian hoạt động
void sendUptime() {

  uptime++;

  Blynk.virtualWrite(V0, uptime);

  display.showNumberDec(uptime, true);
}

//////////////////////////////////////////////////

// Điều khiển LED từ điện thoại
BLYNK_WRITE(V3) {

  ledState = param.asInt();

  digitalWrite(LED_PIN, ledState);
}

//////////////////////////////////////////////////

// Kiểm tra nút nhấn
void checkButton() {

  if (digitalRead(BUTTON_PIN) == LOW) {

    ledState = !ledState;

    digitalWrite(LED_PIN, ledState);

    Blynk.virtualWrite(V3, ledState);

    delay(300); // chống dội
  }
}

//////////////////////////////////////////////////

void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  display.setBrightness(7);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, readSensor);
  timer.setInterval(1000L, sendUptime);
  timer.setInterval(200L, checkButton);
}

//////////////////////////////////////////////////

void loop() {

  Blynk.run();
  timer.run();
}