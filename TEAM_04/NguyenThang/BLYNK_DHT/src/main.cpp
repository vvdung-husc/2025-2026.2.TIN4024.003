#define BLYNK_TEMPLATE_ID "TMPL63Dc-FTxU"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "iOYpmxJtc3XgYwRT8taGmm0YC8A-EVcl"


#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>


char ssid[] = "Wokwi-GUEST";
char pass[] = "";

/* GPIO theo diagram.json */

#define DHTPIN 16
#define DHTTYPE DHT22

#define LED_PIN 21
#define BUTTON_PIN 23

#define CLK 18
#define DIO 19

DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);

BlynkTimer timer;

int uptime = 0;
bool ledState = false;

/* đọc sensor */

void sendSensor()
{
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(humidity) || isnan(temp)) {
    Serial.println("DHT error");
    return;
  }

  Serial.print("Temp: ");
  Serial.println(temp);

  Serial.print("Humidity: ");
  Serial.println(humidity);

  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, humidity);
}

/* cập nhật display */

void updateDisplay()
{
  uptime++;
  display.showNumberDec(uptime);
  Blynk.virtualWrite(V2, uptime);
}

/* điều khiển LED từ app */

BLYNK_WRITE(V3)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);
}

/* đọc button local */

void checkButton()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V3, ledState);
    delay(300);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(7);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensor);
  timer.setInterval(1000L, updateDisplay);
  timer.setInterval(200L, checkButton);
}

void loop()
{
  Blynk.run();
  timer.run();
}