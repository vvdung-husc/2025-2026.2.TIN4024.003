#define BLYNK_TEMPLATE_ID "TMPL6PTRj1_bO"
#define BLYNK_TEMPLATE_NAME "TrafficLight"
#define BLYNK_AUTH_TOKEN "LVifwkLioNZY4SZjlleIUrFacV3nAuCu"

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>
#include <DHT.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define BUTTON_PIN 23
#define LED_PIN 21
#define CLK 18
#define DIO 19
#define DHTPIN 16
#define DHTTYPE DHT22

TM1637Display display(CLK, DIO);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

enum State { GREEN, YELLOW, RED };
State currentState = GREEN;

bool systemOn = true;
bool modeAuto = true;
int counter = 10;

void changeState() {
  if (currentState == GREEN) {
    currentState = YELLOW;
    counter = 3;
  } 
  else if (currentState == YELLOW) {
    currentState = RED;
    counter = 7;
  } 
  else {
    currentState = GREEN;
    counter = 10;
  }
}

void trafficLogic() {

  if (!systemOn) {
    digitalWrite(LED_PIN, LOW);
    display.clear();
    Blynk.virtualWrite(V4, "OFF");
    return;
  }

  if (modeAuto) {
    counter--;
    if (counter <= 0) {
      changeState();
    }
  }

  display.showNumberDec(counter);

  Blynk.virtualWrite(V4, String(counter) + " s");

  if (currentState == RED) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(V1, temperature); 
    Blynk.virtualWrite(V2, humidity);    
  }
}

BLYNK_WRITE(V0) {
  systemOn = param.asInt();
}

BLYNK_WRITE(V3) {
  modeAuto = param.asInt();
}

void checkButton() {
  static bool lastState = HIGH;
  bool current = digitalRead(BUTTON_PIN);

  if (current == LOW && lastState == HIGH) {
    if (!modeAuto && systemOn) {
      changeState();
    }
    delay(200); 
  }

  lastState = current;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, trafficLogic);
  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(100L, checkButton);
}

void loop() {
  Blynk.run();
  timer.run();
}