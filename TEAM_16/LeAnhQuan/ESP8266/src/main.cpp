#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

#define LED_BUILTIN 2

#define DHTPIN 0
#define DHTTYPE DHT22

#define OLED_SDA 4  
#define OLED_SCL 5  

DHT dht(DHTPIN, DHTTYPE);

// OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// LED BLINK TIMER 
unsigned long lastLedTime = 0;
bool ledState = false;

//  LED BLINK FUNCTION
void blinkLED()
{
  if (millis() - lastLedTime >= 1000)
  {
    lastLedTime = millis();

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
  }
}

void setup()
{
  Serial.begin(115200);

  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("ESP8266 start");

  Wire.begin(OLED_SDA, OLED_SCL);

  oled.begin();
  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Truong DHKH");
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "IoT - Nhom 16");

  oled.sendBuffer();

  delay(3000);
}

void loop()
{
  blinkLED();   

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);

  oled.drawStr(0, 12, "ESP8266 SENSOR");

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Doc cam bien that bai!");
    oled.drawStr(0, 40, "Sensor Error");
  }
  else
  {
    Serial.print("Nhiet do: ");
    Serial.print(temperature);
    Serial.print(" C | Do am: ");
    Serial.print(humidity);
    Serial.println(" %");

    char tempStr[20];
    sprintf(tempStr, "Temp: %.1f C", temperature);
    oled.drawStr(0, 35, tempStr);

    char humStr[20];
    sprintf(humStr, "Hum : %.1f %%", humidity);
    oled.drawStr(0, 55, humStr);
  }

  oled.sendBuffer();

  delay(2000);  
}