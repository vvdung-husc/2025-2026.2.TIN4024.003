#include <Arduino.h>
#include <DHT.h>

#include <Wire.h>
#include <U8g2lib.h>

#define LED_BUILTIN 2   // GPIO2

#define DHTPIN 0
#define DHTTYPE DHT22

#define OLED_SDA 4  //D2
#define OLED_SCL 5  //D1

DHT dht(DHTPIN, DHTTYPE);

// Khởi tạo OLED SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

bool IsReady(unsigned long &ulTimer, uint32_t millisecond) {
  if (millis() - ulTimer < millisecond) return false;
  ulTimer = millis();
  return true;
}

bool WelcomeDisplayTimeout(uint msSleep = 5000){
  static ulong lastTimer = 0;
  static bool bDone = false;
  if (bDone) return true;
  if (!IsReady(lastTimer, msSleep)) return false;
  bDone = true;    
  return bDone;
}

void setup() {
  
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("ESP8266 start");

Wire.begin(OLED_SDA, OLED_SCL);  // SDA, SCL

  oled.begin();
  oled.clearBuffer();
  
  oled.setFont(u8g2_font_unifont_t_vietnamese1);
  oled.drawUTF8(0, 14, "Trường ĐHKH");  
  oled.drawUTF8(0, 28, "Khoa CNTT");
  oled.drawUTF8(0, 42, "IoT - Nhóm 8");  

  oled.sendBuffer();  
}

void led_blink() {
  digitalWrite(LED_BUILTIN, LOW);  // bật LED
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH); // tắt LED
  delay(1000);
}
void loop() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Doc cam bien that bai!");
    delay(2000);
    return;
  }

  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Do am: ");
  Serial.print(humidity);
  Serial.println(" %");

  oled.clearBuffer();

  oled.setFont(u8g2_font_unifont_t_vietnamese1);

  char tempStr[30];
  char humStr[30];

  sprintf(tempStr, "Nhiet do: %.1f C", temperature);
  sprintf(humStr, "Do am: %.1f %%", humidity);

  oled.drawUTF8(0, 20, tempStr);
  oled.drawUTF8(0, 40, humStr);

  oled.sendBuffer();

  delay(2000); // DHT22 cần >=2s
}