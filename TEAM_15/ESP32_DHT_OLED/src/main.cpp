/*
Thông tin nhóm 15:
1. Lê Tăng Phước
*/

#include "main.h"
#include "ultils.h"

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setupPins() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_CYAN, OUTPUT);
}

void setup() {
  Serial.begin(115200);

  setupPins();
  dht.begin();

  /* I2C THEO SO DO: SDA = 13, SCL = 12 */
  Wire.begin(13, 12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("DHT read failed");
    delay(2000);
    return;
  }

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C | Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  showOLED(temp, hum);
  updateLED(temp);

  delay(2000);
}
