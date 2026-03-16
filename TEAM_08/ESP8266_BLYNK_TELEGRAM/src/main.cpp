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