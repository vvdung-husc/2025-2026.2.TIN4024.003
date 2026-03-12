#define BLYNK_TEMPLATE_ID "TMPL6dTZ_kmtH"
#define BLYNK_TEMPLATE_NAME "ESMART"
#define BLYNK_AUTH_TOKEN "bP7dycWlHq9SsjnXaIy_Ssu2zjeMirdr"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

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
  }
}