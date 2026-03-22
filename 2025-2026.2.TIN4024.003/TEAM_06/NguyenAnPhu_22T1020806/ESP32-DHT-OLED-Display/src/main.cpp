#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHT.h>

/* =======================
   PIN DEFINITIONS
   ======================= */
#define DHT_PIN     16
#define DHT_TYPE    DHT22

#define LED_GREEN   23
#define LED_YELLOW  18
#define LED_RED     19

#define OLED_SDA    21
#define OLED_SCL    22
#define OLED_ADDR   0x3C

/* =======================
   GLOBAL OBJECTS
   ======================= */
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

/* =======================
   TIMING VARIABLES
   ======================= */
unsigned long lastDhtCheck = 0;
unsigned long lastLedCheck = 0;
unsigned long lastBlinkToggle = 0;

bool blinkState = false;

/* =======================
   DATA STRUCT (JSON alternative)
   ======================= */
struct AtmosphereStatus {
  float temperature;
  float humidity;
  String tempStatus;
};

/* =======================
   FUNCTION DECLARATIONS
   ======================= */
bool isReady(unsigned long &lastTime, unsigned long interval);
AtmosphereStatus getAtmosphereStatus();
void controlLeds(const AtmosphereStatus &status);
void updateOled(const AtmosphereStatus &status);

/* =======================
   SETUP
   ======================= */
void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true);
  }

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
}

/* =======================
   LOOP
   ======================= */
void loop() {
  static AtmosphereStatus currentStatus;

  if (isReady(lastDhtCheck, 100)) {
    currentStatus = getAtmosphereStatus();
    updateOled(currentStatus);
  }

  if (isReady(lastLedCheck, 100)) {
    controlLeds(currentStatus);
  }
}

/* =======================
   FUNCTIONS
   ======================= */

// millis()-based scheduler
bool isReady(unsigned long &lastTime, unsigned long interval) {
  unsigned long now = millis();
  if (now - lastTime >= interval) {
    lastTime = now;
    return true;
  }
  return false;
}

// Read DHT + determine temperature status
AtmosphereStatus getAtmosphereStatus() {
  AtmosphereStatus status;

  status.temperature = dht.readTemperature();
  status.humidity = dht.readHumidity();

  if (isnan(status.temperature) || isnan(status.humidity)) {
    status.tempStatus = "SENSOR ERROR";
    return status;
  }

  float t = status.temperature;

  if (t < 13) status.tempStatus = "TOO COLD";
  else if (t < 20) status.tempStatus = "COLD";
  else if (t < 25) status.tempStatus = "COOL";
  else if (t < 30) status.tempStatus = "WARM";
  else if (t < 35) status.tempStatus = "HOT";
  else status.tempStatus = "TOO HOT";

  return status;
}

// LED behavior controller
void controlLeds(const AtmosphereStatus &status) {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  bool blinking = false;
  int activeLed = -1;

  if (status.tempStatus == "TOO COLD") {
    blinking = true;
    activeLed = LED_GREEN;
  }
  else if (status.tempStatus == "COLD") {
    digitalWrite(LED_GREEN, HIGH);
    return;
  }
  else if (status.tempStatus == "COOL") {
    blinking = true;
    activeLed = LED_YELLOW;
  }
  else if (status.tempStatus == "WARM") {
    digitalWrite(LED_YELLOW, HIGH);
    return;
  }
  else if (status.tempStatus == "HOT") {
    blinking = true;
    activeLed = LED_RED;
  }
  else if (status.tempStatus == "TOO HOT") {
    digitalWrite(LED_RED, HIGH);
    return;
  }

  if (blinking && activeLed != -1) {
    if (isReady(lastBlinkToggle, 500)) {
      blinkState = !blinkState;
    }
    digitalWrite(activeLed, blinkState ? HIGH : LOW);
  }
}

// OLED display
void updateOled(const AtmosphereStatus &status) {
  oled.clearDisplay();
  oled.setCursor(0, 0);

  oled.print("Temp: ");
  oled.print(status.temperature, 1);
  oled.println(" C");

  oled.print("Humidity: ");
  oled.print(status.humidity, 1);
  oled.println(" %");

  oled.println();
  oled.print("Status: ");
  oled.println(status.tempStatus);

  oled.display();
}
