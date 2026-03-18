/*
THÔNG TIN NHÓM 08
1. Trương Hữu Ngọc
2.
*/
#define BLYNK_TEMPLATE_ID "TMPL6mcSAdOVC"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "Za4JrX_t85HA-Iu7JmyC-fMas7kf5_3_"

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// =============================================
//   ESP8266 + OLED SSD1306 + DHT22
//   Hiển thị: Uptime, Nhiệt độ, Độ ẩm
// =============================================

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <DHT.h>
#include <U8g2lib.h>

// ===== PIN CONFIG =====
#define DHTPIN D4
#define DHTTYPE DHT22

// OLED I2C (SCL=D1, SDA=D2)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ===== OBJECT =====
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// ===== BIẾN =====
unsigned long uptimeSeconds = 0;
float temperature = 0;
float humidity = 0;

// ===== TĂNG UPTIME =====
void updateUptime()
{
  uptimeSeconds++;
}

// ===== ĐỌC DHT + GỬI BLYNK =====
void readAndSendDHT()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("DHT loi!");
    return;
  }

  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" C | Do am: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V0, uptimeSeconds);
}

// ===== HIỂN THỊ OLED =====
void displayOnOLED()
{
  u8g2.clearBuffer();

  // Tiêu đề
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "IoT - Team X");

  // Uptime
  u8g2.setFont(u8g2_font_7x14_tf);
  u8g2.setCursor(0, 25);
  u8g2.print("Uptime: ");
  u8g2.print(uptimeSeconds);
  u8g2.print(" s");

  // Nhiệt độ
  u8g2.setFont(u8g2_font_logisoso16_tf);
  u8g2.setCursor(0, 45);
  u8g2.print(temperature, 1);
  u8g2.print(" C");

  // Độ ẩm
  u8g2.setCursor(0, 62);
  u8g2.print(humidity, 1);
  u8g2.print(" %");

  u8g2.sendBuffer();
}

// ===== SETUP =====
void setup()
{
  Serial.begin(115200);

  dht.begin();

  // OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 30, "Khoi dong...");
  u8g2.sendBuffer();

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Timer
  timer.setInterval(1000L, updateUptime);
  timer.setInterval(2000L, readAndSendDHT);
  timer.setInterval(3000L, displayOnOLED);

  Serial.println("He thong san sang!");
}

// ===== LOOP =====
void loop()
{
  Blynk.run();
  timer.run();
}

/*
#define BLYNK_TEMPLATE_ID "TMPL6mcSAdOVC"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "Za4JrX_t85HA-Iu7JmyC-fMas7kf5_3_"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
#define DHTPIN 0          // D3 (GPIO0)
#define DHTTYPE DHT22
#define OLED_SDA 4        // D2 (GPIO4)
#define OLED_SCL 5        // D1 (GPIO5)
#define LED_PIN 2         // D4 (GPIO2 - LED BUILTIN)
#define BTN_PIN 14        // D5 (GPIO14) - Chân an toàn cho nút nhấn

// ===== OBJECTS =====
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

// ===== BIẾN HỆ THỐNG =====
unsigned long currentMiliseconds = 0;
unsigned long uptimeSeconds = 0;
bool counterActive = true;
bool showTemperature = true;

// Cấu trúc hàm kiểm tra thời gian
bool IsReady(unsigned long &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void updateSystemState() {
  // LED BUILTIN của ESP8266 thường là Active LOW (Nối cực dương, 0V mới sáng)
  // Tuy nhiên để khớp logic cũ của bạn, tôi giữ counterActive ? HIGH : LOW
  digitalWrite(LED_PIN, counterActive ? LOW : HIGH); // LOW là Sáng trên ESP8266
  Blynk.virtualWrite(V4, counterActive);
}

void updateCounterButton() {
  static unsigned long lastTime = 0;
  static int lastValue = HIGH;
  if (!IsReady(lastTime, 50)) return;

  int v = digitalRead(BTN_PIN);
  if (v == lastValue) return;
  lastValue = v;

  if (v == LOW) { // Nhấn nút (vì dùng INPUT_PULLUP)
    counterActive = !counterActive;
    Serial.println(counterActive ? "Counter ON" : "Counter OFF");
    updateSystemState();
  }
}

void refreshOLED(float t, float h) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hệ Thống: " + String(counterActive ? "ON" : "OFF"));

  display.setCursor(0, 20);
  if (showTemperature) {
    display.setTextSize(2);
    display.print("Temp: ");
    display.print(t, 1);
    display.print(" C");
  } else {
    display.setTextSize(2);
    display.print("Time: ");
    display.print(uptimeSeconds);
    display.print(" s");
  }
  display.display();
}

void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h);
    if (counterActive) refreshOLED(t, h);
  }
}

void countTime() {
  if (!counterActive) return;
  uptimeSeconds++;
  Blynk.virtualWrite(V0, uptimeSeconds);
}

void toggleDisplay() {
  showTemperature = !showTemperature;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Khởi tạo I2C cho OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  dht.begin();
  display.clearDisplay();
  display.display();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.syncVirtual(V4);
  updateSystemState();

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, countTime);
  timer.setInterval(5000L, toggleDisplay);
}

void loop() {
  Blynk.run();
  timer.run();
  currentMiliseconds = millis();
  updateCounterButton();
}

BLYNK_WRITE(V4) {
  counterActive = param.asInt();
  updateSystemState();
}
*/