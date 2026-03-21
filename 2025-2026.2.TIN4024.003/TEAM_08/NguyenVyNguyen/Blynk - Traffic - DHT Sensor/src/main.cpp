#define BLYNK_TEMPLATE_ID "TMPL6q4aBCj-Y"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic"
#define BLYNK_AUTH_TOKEN "wQQgAtcOC8zVoZc_Ciza4JLd5zxolWh3"

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
  Blynk.virtualWrite(V2, counterActive); // V2 là Switch
}

// ===== XỬ LÝ NÚT NHẤN =====
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
    Blynk.virtualWrite(V0, t); // V0 là Nhiệt độ
    Blynk.virtualWrite(V1, h); // V1 là Độ ẩm

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
  Blynk.virtualWrite(V3, uptimeSeconds); // V3 là Thời gian

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

  // KẾT NỐI BẰNG IP TRỰC TIẾP ĐỂ BỎ QUA LỖI DNS CỦA WOKWI
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "128.199.144.129", 80);
  
  // Nếu mạng yếu hoặc IP trên bị lỗi, bạn có thể thử đổi thành dòng dưới:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "45.55.96.146", 80);

  Blynk.syncVirtual(V2); // Đồng bộ trạng thái Switch từ server

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
BLYNK_WRITE(V2) // Nhận lệnh từ V2 (Switch)
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