#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <U8g2lib.h>

#define LED_PIN   2   // GPIO2 - LED built-in
#define DHTPIN    0   // GPIO0
#define DHTTYPE   DHT22
#define OLED_SDA  4   // D2
#define OLED_SCL  5   // D1

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

unsigned long appStartTime = 0;
unsigned long lastReadTime = 0;

const unsigned long WELCOME_DURATION = 5000;
const unsigned long SENSOR_INTERVAL  = 2000;

float temperature = 0.0;
float humidity = 0.0;
bool sensorOK = false;

bool isElapsed(unsigned long &previousMillis, unsigned long interval) {
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    return true;
  }
  return false;
}

void blinkLED(unsigned long delayTime = 120) {
  digitalWrite(LED_PIN, LOW);   // LED built-in active LOW
  delay(delayTime);
  digitalWrite(LED_PIN, HIGH);
}

void showWelcomeScreen() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(8, 14, "Trường ĐHKH");
  oled.drawUTF8(18, 30, "Khoa CNTT");
  oled.drawUTF8(10, 46, "IoT - Nhóm 0");
  oled.drawUTF8(0, 62, "ESP8266 + DHT22");
  oled.sendBuffer();
}

void showSensorScreen(float temp, float hum) {
  char line1[32];
  char line2[32];

  snprintf(line1, sizeof(line1), "Nhiet do: %.1f C", temp);
  snprintf(line2, sizeof(line2), "Do am   : %.1f %%", hum);

  oled.clearBuffer();
  oled.setFont(u8g2_font_6x13_tf);
  oled.drawStr(20, 14, "DOC CAM BIEN");
  oled.drawLine(0, 18, 127, 18);
  oled.drawStr(0, 38, line1);
  oled.drawStr(0, 58, line2);
  oled.sendBuffer();
}

void showErrorScreen() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x13_tf);
  oled.drawStr(8, 18, "LOI CAM BIEN DHT22");
  oled.drawStr(6, 38, "Khong doc duoc du lieu");
  oled.drawStr(18, 58, "Kiem tra ket noi");
  oled.sendBuffer();
}

void readSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    sensorOK = false;
    Serial.println("Doc cam bien that bai!");
    showErrorScreen();
    return;
  }

  humidity = h;
  temperature = t;
  sensorOK = true;

  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" C | Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  showSensorScreen(temperature, humidity);
  blinkLED();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);   // tat LED luc dau

  dht.begin();
  Wire.begin(OLED_SDA, OLED_SCL);

  oled.begin();
  showWelcomeScreen();

  appStartTime = millis();

  Serial.println("ESP8266 start");
  Serial.println("Khoi dong thanh cong!");
}

void loop() {
  // Hien man hinh chao trong 5 giay dau
  if (millis() - appStartTime < WELCOME_DURATION) {
    return;
  }

  // Doc DHT22 moi 2 giay
  if (isElapsed(lastReadTime, SENSOR_INTERVAL)) {
    readSensor();
  }

  // Khong can code gi them o day
}