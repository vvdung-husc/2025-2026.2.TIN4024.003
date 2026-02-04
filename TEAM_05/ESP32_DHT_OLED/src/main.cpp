/*
THÔNG TIN NHÓM 5:
1. Lê Gia Huy
2. Nguyễn Phú Tuấn
3. Phan Bá Hóa
4.Dương Hữu Tiến
5. Nguyễn Huỳnh Minh Tiến
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

#define OLED_SDA 13
#define OLED_SCL 12

#define DHTPIN 16     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22 (AM2302)

#define LED_BLUE 15
#define LED_YELLOW 2
#define LED_RED 4

// create an OLED display object connected to I2

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);

String strTemp;
bool ledState = false;
int8_t ledNumber = LED_BLUE;

/* ================== SETUP ================== */
void setup()
{
  // Serial
  Serial.begin(9600);
  Serial.println("ESP32 START");

  // LED
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // DHT
  dht.begin();

  // OLED I2C (ESP32)
  Wire.setPins(OLED_SDA, OLED_SCL);

  // Khởi tạo OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("FAILED TO START OLED");
    while (1);
  }

  delay(2000);
  oled.clearDisplay();

  // Hiển thị chào mừng
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 10);
  oled.println("  IOT");
  oled.println(" Welcome!");
  oled.display();
}


void loop()
{
  // put your main code here, to run repeatedly:
  delay(1000);
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  oled.clearDisplay();

  strTemp = String("Temperature: ");

  if (t < 13.0)
  {
    strTemp += "TOO COLD";
    ledNumber = LED_BLUE;
  }
  else if (t >= 13.0 && t <= 20.0)
  {
    strTemp += "COLD";
    ledNumber = LED_BLUE;
  }
  else if (t > 20.0 && t <= 25.0)
  {
    strTemp += "COOL";
    ledNumber = LED_YELLOW;
  }
  else if (t > 25.0 && t <= 30.0)
  {
    strTemp += "WARM";
    ledNumber = LED_YELLOW;
  }
  else if (t > 30.0 && t < 35.0)
  {
    strTemp += "HOT";
    ledNumber = LED_RED;
  }
  else
  {
    strTemp += "TOO HOT";
    ledNumber = LED_RED;
  }

  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print(strTemp.c_str());
  oled.setTextSize(2);
  oled.setCursor(0, 10);
  oled.print(t);
  oled.print(" ");
  oled.setTextSize(1);
  oled.cp437(true);
  oled.write(167); //*C
  oled.setTextSize(2);
  oled.print("C");

  oled.setTextSize(1);
  oled.setCursor(0, 35);
  oled.print("Humidity: ");
  oled.setTextSize(2);
  oled.setCursor(0, 45);
  oled.print(h);
  oled.print(" %");

  oled.display();

  digitalWrite(ledNumber, HIGH);
  delay(500);
  digitalWrite(ledNumber, LOW);
}
