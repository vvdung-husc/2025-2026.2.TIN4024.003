<<<<<<< HEAD
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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600); // 115200

  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_BLUE, LOW);   // OFF
  digitalWrite(LED_YELLOW, LOW); // OFF
  digitalWrite(LED_RED, LOW);    // OFF

  dht.begin();

  TwoWire *_Wire = &Wire;
  _Wire->setPins(OLED_SDA, OLED_SCL);
  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1)
      ;
  }

  delay(2000);         // wait two seconds for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(2);               // set text size
  oled.setTextColor(WHITE);          // set text color
  oled.setCursor(0, 2);              // set position to display (x,y)
  oled.println("   IOT\n Welcome!"); // set text
  oled.display();                    // display on OLED
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

  if (t < 0.0)
  {
    strTemp += "Too COOL";
    ledNumber = LED_YELLOW;
  }
  else if (t < 40.0)
  {
    strTemp += "Normal";
    ledNumber = LED_BLUE;
  }
  else
  {
    strTemp += "Too HOT";
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
=======
/*
THÔNG TIN NHÓM 5:
1. Lê Gia Huy
2. Nguyễn Phú Tuấn

*/
#include <Arduino.h>
>>>>>>> 55365a551b3206ee7ea7f22abef8a70b83a04453
