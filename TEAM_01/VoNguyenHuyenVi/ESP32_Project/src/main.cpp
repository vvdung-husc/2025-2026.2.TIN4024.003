#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

#define LED 2
#define DHTPIN D4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void setup()
{
    pinMode(LED, OUTPUT);

    Serial.begin(9600);

    dht.begin();

    u8g2.begin();

    delay(2000);   
}

void loop()
{
    digitalWrite(LED, HIGH);
    delay(500);

    digitalWrite(LED, LOW);
    delay(500);

    float temp = dht.readTemperature();

    Serial.print("Temp: ");
    Serial.println(temp);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.drawStr(0,15,"ESP8266 Project");

    char buffer[10];
    dtostrf(temp,4,2,buffer);

    u8g2.drawStr(0,40,"Temp:");
    u8g2.drawStr(50,40,buffer);
    u8g2.drawStr(90,40,"C");

    u8g2.sendBuffer();

    delay(2000);   
}