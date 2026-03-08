#define BLYNK_TEMPLATE_ID "TMPL6AJWRTv7y"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT sensor"
#define BLYNK_AUTH_TOKEN "EZPKS73v1Rey8uiWh1M34pFWEIMu4Thz"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
#define DHTPIN 16
#define DHTTYPE DHT22
#define CLK 18
#define DIO 19
#define LED_PIN 21
#define BTN_PIN 23

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;

// ===== BIẾN HỆ THỐNG =====
unsigned long currentMiliseconds = 0;
unsigned long uptimeSeconds = 0;

bool counterActive = true;   // Trạng thái công tắc
bool showTemperature = true; // Luân phiên hiển thị

// ===== HÀM CHỐNG DỘI GIỐNG CODE MẪU =====
bool IsReady(unsigned long &ulTimer, uint32_t milisecond)
{
  if (currentMiliseconds - ulTimer < milisecond)
    return false;
  ulTimer = currentMiliseconds;
  return true;
}

// ===== ĐỒNG BỘ TRẠNG THÁI =====
void updateSystemState()
{
  digitalWrite(LED_PIN, counterActive ? HIGH : LOW);
  Blynk.virtualWrite(V4, counterActive);
}

// ===== XỬ LÝ NÚT NHẤN (giống updateBlueButton) =====
void updateCounterButton()
{
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;

  if (!IsReady(lastTime, 50))
    return;

  int v = digitalRead(BTN_PIN);
  if (v == lastValue)
    return;

  lastValue = v;
  if (v == LOW)
    return; // xử lý khi nhả nút

  counterActive = !counterActive;

  if (counterActive)
  {
    Serial.println("Counter ON");
  }
  else
  {
    Serial.println("Counter OFF");
  }

  updateSystemState();
}

// ===== ĐỌC CẢM BIẾN =====
void sendSensorData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t))
  {

    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);

    if (showTemperature && counterActive)
    {
      display.showNumberDec((int)t);
    }
  }
}

// ===== ĐẾM THỜI GIAN =====
void countTime()
{
  if (!counterActive)
    return;

  uptimeSeconds++;
  Blynk.virtualWrite(V0, uptimeSeconds);

  if (!showTemperature)
  {
    display.showNumberDec(uptimeSeconds);
  }
}

// ===== LUÂN PHIÊN HIỂN THỊ =====
void toggleDisplay()
{
  showTemperature = !showTemperature;
  display.clear();
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Blynk.syncVirtual(V4); // tránh lệch trạng thái khi reconnect

  updateSystemState();

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, countTime);
  timer.setInterval(5000L, toggleDisplay);

  Serial.println("== SYSTEM READY ==");
}

// ===== LOOP =====
void loop()
{
  Blynk.run();
  timer.run();

  currentMiliseconds = millis();
  updateCounterButton();
}

// ===== NHẬN LỆNH TỪ APP =====
BLYNK_WRITE(V4)
{
  counterActive = param.asInt();

  if (counterActive)
  {
    Serial.println("Blynk -> Counter ON");
  }
  else
  {
    Serial.println("Blynk -> Counter OFF");
  }

  updateSystemState();
}