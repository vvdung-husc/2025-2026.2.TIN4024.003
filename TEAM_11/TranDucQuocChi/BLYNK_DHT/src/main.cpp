#define BLYNK_TEMPLATE_ID "TMPL6dTZ_kmtH"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "bP7dycWlHq9SsjnXaIy_Ssu2zjeMirdr"
<<<<<<< HEAD
=======

/* Comment this out to disable prints and save space */
>>>>>>> f430aa2ed957c40a75ec6694f763baaac60a59d9
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
<<<<<<< HEAD
#include <TM1637Display.h>
#include "DHT.h"

char ssid[] = "Wokwi-GUEST";
char pass[] = "";


// Cấu hình chân Pin
#define LED_PIN 21
#define BTN_PIN 23
#define CLK_PIN 18
#define DIO_PIN 19
#define DHTPIN  16
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK_PIN, DIO_PIN);
BlynkTimer timer;

bool ledState = false;
bool running = false;
unsigned long startMs = 0;
int lastBtn = HIGH;

// Hàm gửi dữ liệu cảm biến lên Blynk mỗi 2 giây
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t); // Gửi nhiệt độ lên V2
    Blynk.virtualWrite(V3, h); // Gửi độ ẩm lên V3
  }
}

void showMMSS(unsigned long sec) {
  int mm = (sec / 60) % 100;
  int ss = sec % 60;
  display.showNumberDecEx(mm * 100 + ss, 0b01000000, true, 4, 0);
  Blynk.virtualWrite(V4, sec); // Gửi thời gian lên V4
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  display.setBrightness(7, true);
  dht.begin();

  // Kết nối qua IP để tránh lỗi DNS trên Wokwi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); Serial.print("."); 
  }
  Blynk.config(BLYNK_AUTH_TOKEN, "128.199.144.129", 80);
  Blynk.connect();
  timer.setInterval(2000L, sendSensorData); // Cứ 2s gửi cảm biến 1 lần
}

void loop() {
  Blynk.run();
  timer.run();
  // Xử lý nút nhấn vật lý
  int currentBtn = digitalRead(BTN_PIN);
  if (lastBtn == HIGH && currentBtn == LOW) {
    ledState = !ledState;
    running = ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V1, ledState); // Đồng bộ trạng thái nút lên App
    if (running) startMs = millis();
    else display.clear();
    delay(200);
  }
  lastBtn = currentBtn;

  if (running) {
    showMMSS((millis() - startMs) / 1000);
  }
}
// Điều khiển từ App/Web xuống
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
  running = ledState;
  if (running) startMs = millis();
  else {
    display.clear();
    Blynk.virtualWrite(V4, 0);
=======

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#include <Arduino.h>
#include <TM1637Display.h>

constexpr int LED_PIN = 21;
constexpr int BTN_PIN = 23;   // INPUT_PULLUP, nhấn = LOW
constexpr int CLK_PIN = 18;
constexpr int DIO_PIN = 19;

TM1637Display display(CLK_PIN, DIO_PIN);

bool running = false;
unsigned long startMs = 0;

// Debounce nhanh
constexpr unsigned long DEBOUNCE_MS = 15;
int rawLast = HIGH;
int stable = HIGH;
unsigned long lastBounceMs = 0;

// Update đồng hồ chỉ khi đổi giây
long lastShownSec = -1;

static inline void showMMSS(unsigned long sec) {
  int mm = (sec / 60) % 100;
  int ss = sec % 60;
  uint16_t value = mm * 100 + ss;
  display.showNumberDecEx(value, 0b01000000, true, 4, 0);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  Blynk.connect();

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7, true);
  digitalWrite(LED_PIN, LOW);
  showMMSS(0);
}

bool ledState = false;

void loop() {
  Blynk.run();

  // ---- Debounce nút ----
  int raw = digitalRead(BTN_PIN);
  if (raw != rawLast) {
    rawLast = raw;
    lastBounceMs = millis();
  }

  if (millis() - lastBounceMs >= DEBOUNCE_MS && raw != stable) {
    int prevStable = stable;
    stable = raw;

    // Bắt cạnh nhấn: HIGH -> LOW
    if (prevStable == HIGH && stable == LOW) {

  ledState = !ledState;          // đảo trạng thái LED
  digitalWrite(LED_PIN, ledState);

  Blynk.virtualWrite(V1, ledState);

  running = ledState;            // đồng hồ chạy khi LED bật

  if (running) {
    startMs = millis();
    lastShownSec = -1;
  } else {
    display.clear();
  }
}
  }

  // ---- Cập nhật đồng hồ (mỗi khi đổi giây) ----
if (running) {
  unsigned long sec = (millis() - startMs) / 1000;
  if ((long)sec != lastShownSec) {
    lastShownSec = (long)sec;

    showMMSS(sec);

    Blynk.virtualWrite(V4, sec);   // gửi thời gian lên app
  }
}
}

BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);

  running = ledState;

  if (running) {
    startMs = millis();
    lastShownSec = -1;
  } else {
    display.clear();
>>>>>>> f430aa2ed957c40a75ec6694f763baaac60a59d9
  }
}