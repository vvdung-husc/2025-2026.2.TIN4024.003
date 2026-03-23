/*
	THÔNG TIN NHÓM X
	1. Trương Trường Phúc
	2. Lê Thanh Nhật
  	3. Hoàng Văn Thắng - Telegram : caubemenhac
  	4. Nguyễn Đức Dương
	5. Phan Văn Hữu Thắng
	*/

#define BLYNK_TEMPLATE_ID "TMPL6NkrRnQam"
#define BLYNK_TEMPLATE_NAME "BLYNK DHT"
#define BLYNK_AUTH_TOKEN "It8AOO-TEuEm0l8mW31LBXU7HrcLk7cI"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ============================================
// WiFi Configuration
// ============================================
char ssid[] = "HELO KITTY"; // Your WiFi SSID
char pass[] = "0777567369";

// ============================================
// Blynk Configuration
// ============================================
// Blynk macros are declared before #include <BlynkSimpleEsp8266.h>

// ============================================
// Telegram Configuration
// ============================================
#define BOT_TOKEN "8670966008:AAG4MipyrwGNU6VS7LbUx7j-pTdVxyYdndY"
#define CHAT_ID "-1003858517058"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ============================================
// OLED Configuration (I2C)
// ============================================
const uint8_t OLED_SDA = D2; // GPIO4
const uint8_t OLED_SCL = D1; // GPIO5

// U8g2 constructor - SH1106 128x64 display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

// ============================================
// DHT Sensor Configuration
// ============================================
#define DHTTYPE DHT22
const uint8_t DHT_PIN = D3; // D3 (GPIO0)
DHT dht(DHT_PIN, DHTTYPE);

// ============================================
// GPIO Configuration
// ============================================
const uint8_t LED_PIN = D8;      // GPIO15 - LED control (safe pin)
const uint8_t GAS_SENSOR_PIN = A0; // Analog input for MQ2 gas sensor

// ============================================
// Timing Configuration
// ============================================
const unsigned long SENSOR_INTERVAL = 5000;     // Read sensors every 5s
const unsigned long OLED_UPDATE_INTERVAL = 2000; // Update OLED every 2s
const unsigned long TELEGRAM_INTERVAL = 1000;   // Check Telegram every 1s
const unsigned long BLYNK_SYNC_INTERVAL = 10000; // Sync with Blynk every 10s
const unsigned long WIFI_RECONNECT_INTERVAL = 10000; // Retry WiFi every 10s
const unsigned long BLYNK_RECONNECT_INTERVAL = 10000; // Retry Blynk every 10s

unsigned long lastSensorMillis = 0;
unsigned long lastOledMillis = 0;
unsigned long lastTelegramMillis = 0;
unsigned long lastBlynkSyncMillis = 0;
unsigned long lastWifiRetryMillis = 0;
unsigned long lastBlynkRetryMillis = 0;

// ============================================
// Global Variables
// ============================================
unsigned long deviceUptime = 0;
float currentTemp = 25.0;
float currentHumidity = 50.0;
int gasValue = 0;
bool ledState = false;
unsigned long lastTempChangeTime = 0;
float lastSentTemp = 25.0;
float lastSentHumidity = 50.0;
unsigned long lastLedV1WriteMillis = 0;

void notifyTelegram(const String& message) {
  if (WiFi.status() == WL_CONNECTED) {
    bool ok = bot.sendMessage(CHAT_ID, message, "");
    Serial.println(ok ? "[Telegram] Message sent" : "[Telegram] Send failed");
  }
}

// ============================================
// Blynk Virtual Pin Handlers
// ============================================

// V1: LED Switch control (read/write)
BLYNK_WRITE(V1) {
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  lastLedV1WriteMillis = millis();  // Record last write time
  
  String msg = "LED turned " + String(ledState ? "ON" : "OFF");
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, msg, "");
    Serial.println("[Telegram] Message sent: " + msg);
  }
  
  Serial.println("[BLYNK] V1 WRITE - " + msg);
}

// V1: LED Switch status (return current state to app)
BLYNK_READ(V1) {
  Blynk.virtualWrite(V1, ledState ? 1 : 0);
  Serial.println("[BLYNK] V1 READ - LED State: " + String(ledState ? "ON" : "OFF"));
}

// ============================================
// Functions
// ============================================

void connectWiFi() {
  if (strlen(ssid) == 0) {
    Serial.println("\n[WiFi] SSID is empty. Please set ssid/pass in source code.");
    return;
  }

  Serial.print("\n[WiFi] Connecting to: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] Connection failed!");
  }
}

void connectBlynk() {
  Serial.println("[Blynk] Connecting...");
  Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
  bool connected = Blynk.connect(3000);
  
  if (connected) {
    Serial.println("[Blynk] Connected!");
    // Sync only sensor virtual pins (not V1 which is controlled output)
    Blynk.syncVirtual(V0, V2, V3, V4);
  } else {
    Serial.println("[Blynk] Connection failed (check token/template/internet)");
  }
}

void readSensors() {
  // Read DHT sensor
  float tempC = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(tempC) && !isnan(hum)) {
    currentTemp = tempC;
    currentHumidity = hum;
    Serial.print("[DHT] Temp: ");
    Serial.print(currentTemp, 1);
    Serial.print("°C, Humidity: ");
    Serial.print(currentHumidity, 1);
    Serial.println("%");
    
    // Write temperature & humidity to Blynk
    if (Blynk.connected()) {
      Blynk.virtualWrite(V2, currentTemp);
      Blynk.virtualWrite(V3, currentHumidity);
      Serial.println("[BLYNK] V2/V3 updated");
    }
  } else {
    Serial.println("[DHT] Failed to read!");
  }
  
  // Read MQ2 Gas sensor (or generate random value if not available)
  int rawGas = analogRead(GAS_SENSOR_PIN);
  // Map analog value (0-1023) to a readable gas value
  gasValue = map(rawGas, 0, 1023, 0, 1000);
  
  // If no sensor connected, generate random value for demo
  if (rawGas < 10) {
    gasValue = random(50, 200);
  }
  
  Serial.print("[MQ2] Gas value: ");
  Serial.println(gasValue);
  
  // Write gas sensor to Blynk
  if (Blynk.connected()) {
    Blynk.virtualWrite(V4, gasValue);
    Serial.println("[BLYNK] V4 updated");
  }
}

void updateOledDisplay() {
  u8g2.clearBuffer();
  
  // Set font for title
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 8, "IOT Device");
  
  // Uptime
  unsigned long uptimeSeconds = millis() / 1000;
  unsigned long hours = uptimeSeconds / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  
  u8g2.setFont(u8g2_font_7x14_tr);
  char uptimeStr[20];
  snprintf(uptimeStr, sizeof(uptimeStr), "Uptime: %02lu:%02lu", hours, minutes);
  u8g2.drawStr(0, 20, uptimeStr);
  
  // LED Switch Status
  u8g2.drawStr(0, 34, "LED: ");
  u8g2.drawStr(45, 34, ledState ? "ON" : "OFF");
  
  // Temperature
  char tempStr[20];
  snprintf(tempStr, sizeof(tempStr), "T: %.1f C", currentTemp);
  u8g2.drawStr(80, 34, tempStr);
  
  // Humidity
  char humStr[20];
  snprintf(humStr, sizeof(humStr), "H: %.1f%%", currentHumidity);
  u8g2.drawStr(0, 48, humStr);
  
  // Gas Sensor
  char gasStr[20];
  snprintf(gasStr, sizeof(gasStr), "Gas: %d ppm", gasValue);
  u8g2.drawStr(0, 62, gasStr);
  
  u8g2.sendBuffer();
  
  Serial.println("[OLED] Display updated");
}

void handleTelegramMessages() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    Serial.println("[Telegram] Received " + String(numNewMessages) + " messages");

    for (int i = 0; i < numNewMessages; i++) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;

      if (chat_id != CHAT_ID) {
        Serial.println("[Telegram] Ignored message from unauthorized chat: " + chat_id);
        continue;
      }
      
      Serial.println("[Telegram] Message: " + text);

      // Normalize command format for group/private chats:
      // supports: on, /on, /on@botname, status, /status@botname
      text.trim();
      text.toLowerCase();

      if (text.startsWith("/")) {
        int spaceIndex = text.indexOf(' ');
        String commandToken = (spaceIndex > 0) ? text.substring(0, spaceIndex) : text;

        int atIndex = commandToken.indexOf('@');
        if (atIndex > 0) {
          commandToken = commandToken.substring(0, atIndex);
        }

        text = commandToken.substring(1);
      }
      
      String reply = "";
      
      // Commands
      if (text == "on") {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
        reply = "✓ LED turned ON";
        Blynk.virtualWrite(V1, 1);
      } 
      else if (text == "off") {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
        reply = "✓ LED turned OFF";
        Blynk.virtualWrite(V1, 0);
      } 
      else if (text == "status") {
        reply = "Device Status:\n";
        reply += "LED: " + String(ledState ? "ON" : "OFF") + "\n";
        reply += "Temp: " + String(currentTemp, 1) + "°C\n";
        reply += "Humidity: " + String(currentHumidity, 1) + "%\n";
        reply += "Gas: " + String(gasValue) + " ppm\n";
        
        unsigned long uptimeSeconds = millis() / 1000;
        unsigned long hours = uptimeSeconds / 3600;
        unsigned long minutes = (uptimeSeconds % 3600) / 60;
        reply += "Uptime: " + String(hours) + "h " + String(minutes) + "m";
      } 
      else if (text == "start") {
        reply = "✓ Bot started!\nCommands:\n";
        reply += "- /on or on (turn LED on)\n";
        reply += "- /off or off (turn LED off)\n";
        reply += "- /status or status (device info)";
      }
      else {
        reply = "❌ Command not recognized.\nTry: /on, /off, /status, /start";
      }
      
      bot.sendMessage(chat_id, reply, "");
      Serial.println("[Telegram] Reply sent");
    }

    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void checkTempHumidityChange() {
  // Send temperature/humidity info to Telegram if changed significantly
  if (abs(currentTemp - lastSentTemp) >= 1.0 || abs(currentHumidity - lastSentHumidity) >= 5.0) {
    String msg = "🌡️ Sensor Update:\n";
    msg += "Temperature: " + String(currentTemp, 1) + "°C\n";
    msg += "Humidity: " + String(currentHumidity, 1) + "%";
    
    notifyTelegram(msg);
    
    lastSentTemp = currentTemp;
    lastSentHumidity = currentHumidity;
  }
}

// ============================================
// Setup
// ============================================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=====================================");
  Serial.println("ESP8266 IoT Project - Starting...");
  Serial.println("=====================================\n");
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize I2C and OLED
  Serial.println("[OLED] Initializing...");
  Wire.begin(OLED_SDA, OLED_SCL);
  delay(100);
  
  if (u8g2.begin()) {
    Serial.println("[OLED] Initialized successfully!");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 8, "ESP8266 IoT");
    u8g2.drawStr(0, 25, "Initializing...");
    u8g2.sendBuffer();
  } else {
    Serial.println("[OLED] Initialization failed!");
  }
  
  // Initialize DHT sensor
  Serial.println("[DHT] Initializing...");
  dht.begin();
  delay(2000);
  
  // Read DHT once to verify
  float testTemp = dht.readTemperature();
  float testHum = dht.readHumidity();
  if (!isnan(testTemp) && !isnan(testHum)) {
    Serial.println("[DHT] Initialized successfully!");
  } else {
    Serial.println("[DHT] Initialization failed - check wiring!");
  }
  
  // Connect to WiFi
  connectWiFi();
  
  // Configure Telegram bot
  client.setInsecure();
  
  // Connect to Blynk
  connectBlynk();
  
  Serial.println("\n[Setup] Complete!\n");
}

// ============================================
// Main Loop
// ============================================
void loop() {
  unsigned long now = millis();

  // Keep WiFi connected before handling cloud services
  if (WiFi.status() != WL_CONNECTED && (now - lastWifiRetryMillis >= WIFI_RECONNECT_INTERVAL)) {
    lastWifiRetryMillis = now;
    connectWiFi();
  }
  
  // Maintain Blynk connection
  if (WiFi.status() == WL_CONNECTED) {
    if (Blynk.connected()) {
      Blynk.run();
    } else if (now - lastBlynkRetryMillis >= BLYNK_RECONNECT_INTERVAL) {
      lastBlynkRetryMillis = now;
      connectBlynk();
    }
  }
  
  // Read sensors
  if (now - lastSensorMillis >= SENSOR_INTERVAL) {
    lastSensorMillis = now;
    readSensors();
    checkTempHumidityChange();
  }
  
  // Update OLED display
  if (now - lastOledMillis >= OLED_UPDATE_INTERVAL) {
    lastOledMillis = now;
    updateOledDisplay();
  }
  
  // Check Telegram messages
  if (now - lastTelegramMillis >= TELEGRAM_INTERVAL) {
    lastTelegramMillis = now;
    if (WiFi.status() == WL_CONNECTED) {
      handleTelegramMessages();
    }
  }
  
  // Sync with Blynk
  if (now - lastBlynkSyncMillis >= BLYNK_SYNC_INTERVAL) {
    lastBlynkSyncMillis = now;
    if (Blynk.connected()) {
      // Write uptime to Blynk (as integer seconds)
      unsigned long uptimeSeconds = millis() / 1000;
      Blynk.virtualWrite(V0, (int)uptimeSeconds);
      Serial.println("[BLYNK] V0 (Uptime) updated: " + String(uptimeSeconds) + " seconds");
    }
  }
  
  delay(10);
}
