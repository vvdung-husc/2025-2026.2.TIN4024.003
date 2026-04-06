#define BLYNK_TEMPLATE_ID "TMPL61fTeFhsM"
#define BLYNK_TEMPLATE_NAME "BLYNK"
#define BLYNK_AUTH_TOKEN "DUDSAG3JDZWfT7djkrUWbVdnsaNI8miq"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define DHTPIN 16     
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define CLK 18        
#define DIO 19
TM1637Display display(CLK, DIO);

#define LED_PIN 21    
#define BTN_PIN 23    

BlynkTimer timer;

bool isLightOn = false;
int activeTime = 0; 
int lastButtonState = HIGH;

void sendSensorData();
void handleStopwatch();
void checkPhysicalButton();

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  display.setBrightness(0x0f); 
  display.clear();
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, sendSensorData);   
  timer.setInterval(1000L, handleStopwatch);    
  timer.setInterval(100L, checkPhysicalButton); 
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, t); 
    Blynk.virtualWrite(V3, h); 
  }
}

BLYNK_WRITE(V1) {
  int switchState = param.asInt();
  
  if (switchState == 1) {
    isLightOn = true;
    activeTime = 0;              
    digitalWrite(LED_PIN, HIGH); 
    display.showNumberDec(activeTime); 
    Blynk.virtualWrite(V0, activeTime); 
  } else {
    isLightOn = false;
    digitalWrite(LED_PIN, LOW);  
    display.clear();     
    activeTime = 0;        
    Blynk.virtualWrite(V0, activeTime); 
  }
}

void checkPhysicalButton() {
  int currentButtonState = digitalRead(BTN_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (!isLightOn) {
      isLightOn = true;
      activeTime = 0;
      digitalWrite(LED_PIN, HIGH);
      display.showNumberDec(activeTime);
      Blynk.virtualWrite(V1, 1); 
      Blynk.virtualWrite(V0, activeTime); 
    } else {
      isLightOn = false;
      digitalWrite(LED_PIN, LOW);
      display.clear();
      Blynk.virtualWrite(V1, 0); 
      activeTime = 0;
      Blynk.virtualWrite(V0, activeTime); 
    }
  }
  lastButtonState = currentButtonState; 
}

void handleStopwatch() {
  if (isLightOn) {
    activeTime++; 
    display.showNumberDec(activeTime); 
    Blynk.virtualWrite(V0, activeTime); 
  }
}