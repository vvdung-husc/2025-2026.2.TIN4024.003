#define BLYNK_TEMPLATE_ID "TMPL6gTDMaemN"
#define BLYNK_TEMPLATE_NAME "blynk"
#define BLYNK_AUTH_TOKEN "pezh6zi8tC3Wx2-XJBxV18N3oq0KartB"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

// WIFI
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// PIN
#define LED_PIN 21
#define BTN_PIN 23

#define CLK 18
#define DIO 19

#define DHT_PIN 16
#define DHT_TYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHT_PIN, DHT_TYPE);

BlynkTimer timer;

// trạng thái
bool ledState = false;
int lastButton = HIGH;

unsigned long startTime = 0;
unsigned long runTime = 0;

// ===== BLYNK SWITCH =====
BLYNK_WRITE(V0) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);

  if (ledState) startTime = millis();
}

// ===== SENSOR =====
void sendSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t) && !isnan(h)) {
    Blynk.virtualWrite(V2, t);
    Blynk.virtualWrite(V3, h);
  }
}

// ===== TIMER DISPLAY =====
void updateClock() {
  if (ledState) {
    runTime = (millis() - startTime) / 1000;
  }

  display.showNumberDec(runTime, true);
  //Blynk.virtualWrite(V1, runTime);
  static int lastTime = -1;

  if (runTime != lastTime) {
    Blynk.virtualWrite(V1, runTime);
    lastTime = runTime;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  display.setBrightness(7);

  dht.begin();
//Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  WiFi.begin(ssid, pass);

while (WiFi.status() != WL_CONNECTED) {
  delay(100);
}

Blynk.config(BLYNK_AUTH_TOKEN);
Blynk.connect();//

  // timer tối ưu
  timer.setInterval(1000L, updateClock);  // đồng hồ 1s
  //timer.setInterval(2000L, sendSensor);   // sensor 2s
  timer.setInterval(5000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();
  //
  yield();
  //
  // button local
  int buttonState = digitalRead(BTN_PIN);

  if (buttonState == LOW && lastButton == HIGH) {
    ledState = !ledState;

    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V0, ledState);

    if (ledState) startTime = millis();
  }

  lastButton = buttonState;
}