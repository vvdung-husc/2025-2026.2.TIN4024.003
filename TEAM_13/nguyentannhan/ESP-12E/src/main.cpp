#include <Arduino.h>
#include <TM1637Display.h>
#include <DHT.h>

/* ===== Khai báo chân ===== */

#define LED_PIN 2

#define DHTPIN 4
#define DHTTYPE DHT11

#define CLK 12
#define DIO 14

/* ===== Khởi tạo đối tượng ===== */

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

/* ===== Biến thời gian ===== */

unsigned long ledTimer = 0;
unsigned long sensorTimer = 0;
unsigned long displayTimer = 0;

bool ledState = false;
bool showTemp = true;

float temperature = 0;
float humidity = 0;

/* ===== SETUP ===== */

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  dht.begin();

  display.setBrightness(7);
  display.clear();

  delay(2000); // ổn định cảm biến
}

/* ===== LOOP ===== */

void loop()
{
  unsigned long now = millis();

  /* LED nhấp nháy */
  if (now - ledTimer >= 500)
  {
    ledTimer = now;

    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

  /* Đọc DHT22 mỗi 3s */
  if (now - sensorTimer >= 3000)
  {
    sensorTimer = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      temperature = t;
      humidity = h;

      Serial.print("Temperature: ");
      Serial.println(temperature);

      Serial.print("Humidity: ");
      Serial.println(humidity);
    }
    else
    {
      Serial.println("Loi doc DHT22");
    }
  }

  /* Hiển thị lên TM1637 */
  if (now - displayTimer >= 3000)
  {
    displayTimer = now;

    if (showTemp)
    {
      int tempDisplay = temperature * 10;
      display.showNumberDecEx(tempDisplay, 0b01000000, true);
    }
    else
    {
      int humDisplay = humidity * 10;
      display.showNumberDecEx(humDisplay, 0b01000000, true);
    }

    showTemp = !showTemp;
  }
}