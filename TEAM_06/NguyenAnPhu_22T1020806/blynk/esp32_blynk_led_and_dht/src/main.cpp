/* 1. BLYNK DECLARATIONS (Must come first) */
#define BLYNK_TEMPLATE_ID "TMPL6_9LuILHZ"
#define BLYNK_TEMPLATE_NAME "esp32 LED and DHT sensor"
#define BLYNK_AUTH_TOKEN "mXYGFUVg5fFVt52H8Jy8ypiJTFTaV0tG"

/* 2. LIBRARIES */
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>

/* 3. VARIABLES & DEFINITIONS */
// Hardware Pins based on your diagram.json
#define PIN_DHT 16
#define PIN_CLK 18
#define PIN_DIO 19
#define PIN_LED 21
#define PIN_BTN 23

// Constants
#define DHTTYPE DHT22
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST"; // Standard Wokwi WiFi
char pass[] = "";

// Object Initialization
DHT dht(PIN_DHT, DHTTYPE);
TM1637Display display(PIN_CLK, PIN_DIO);
BlynkTimer timer;

// State Variables
int ledState = LOW;
int lastButtonState = HIGH;
unsigned long startTime;

/* 4. SUPPORT FUNCTION DECLARATIONS */
void sendSensorData();
void updateElapsedTime();
void checkButton();

/* 5. SETUP & LOOP */
void setup() {
  Serial.begin(115200);

  // Pin Modes
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP); // Using internal pullup for the button

  // Initialize Components
  dht.begin();
  display.setBrightness(0x0f);
  startTime = millis();

  // Blynk Setup (specify current server and port)
  Blynk.begin(auth, ssid, pass, "sgp1.blynk.cloud", 80);

  // Setup Timers (To avoid flooding Blynk servers)
  timer.setInterval(2000L, sendSensorData);    // Every 2 seconds
  timer.setInterval(1000L, updateElapsedTime); // Every 1 second
}

void loop() {
  Blynk.run();
  timer.run();
  checkButton(); // Checked every loop for responsiveness
}

/* 6. DETAILED FUNCTION DEFINITIONS */

/**
 * Reads Temperature and Humidity from DHT22
 * Sends data to Blynk Virtual Pins V3 and V4
 */
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V3, (int)t);
  Blynk.virtualWrite(V4, (int)h);
  
  Serial.print("Temp: "); Serial.print(t);
  Serial.print(" Humidity: "); Serial.println(h);
}

/**
 * Calculates time since the program started in seconds
 * Updates the 4-digit display and Blynk Virtual Pin V0
 */
void updateElapsedTime() {
  long elapsedSeconds = (millis() - startTime) / 1000;
  
  // Cap at 9999 as per your requirement
  if (elapsedSeconds > 9999) elapsedSeconds = 9999;

  display.showNumberDec(elapsedSeconds, false);
  Blynk.virtualWrite(V0, elapsedSeconds);
}

/**
 * Monitors the physical button for a toggle action
 * Toggles the LED and updates Blynk Virtual Pin V1
 */
void checkButton() {
  int currentButtonState = digitalRead(PIN_BTN);

  // Simple state change detection (Pressed is LOW because of PULLUP)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState);
    
    // Update Blynk
    Blynk.virtualWrite(V1, ledState);
    Serial.print("LED Toggled: "); Serial.println(ledState);
    
    delay(50); // Small debounce
  }
  lastButtonState = currentButtonState;
}