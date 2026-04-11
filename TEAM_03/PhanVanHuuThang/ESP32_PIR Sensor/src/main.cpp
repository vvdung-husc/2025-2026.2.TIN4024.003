/***************************
   ESP32 Motion Alarm System
   - PIR Sensor: GPIO 27
   - LED Alert: GPIO 21
   - WiFi: Blynk + Telegram
   - Author: Phan Văn Hữu Thắng
 ***************************/

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6SEoh8hhp"
#define BLYNK_TEMPLATE_NAME "ESP32 PIR SENSOR"
#define BLYNK_AUTH_TOKEN "Iy2i77YzeabXzAzwc7LDzOZEWh8yBJ9a"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <time.h>

// ========== WiFi Configuration ==========
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ========== Telegram Configuration ==========
#define BOT_TOKEN "8729067566:AAF92vtaNlMUTBILmI9R2AVH0PC4TX2A6FE"
#define CHAT_ID "5916461003"

// ========== Pin Configuration ==========
#define PIR_SENSOR_PIN 27     // PIR OUT pin
#define LED_ALERT_PIN 21      // LED pin
#define BUZZER_PIN 25         // Optional: Buzzer pin

// ========== System Variables ==========
bool motionDetected = false;
bool systemArmed = true;
unsigned long lastMotionTime = 0;
unsigned long lastTelegramTime = 0;
unsigned long lastBlynkUpdateTime = 0;
unsigned long lastMetricWriteTime = 0;
unsigned long lastBlynkReconnectAttempt = 0;
int alertCount = 0;

const unsigned long TELEGRAM_INTERVAL = 30000;  // 30 seconds min between Telegram messages
const unsigned long BLYNK_INTERVAL = 1000;      // 1 second for basic dashboard updates
const unsigned long METRIC_WRITE_INTERVAL_MS = 60000; // Write chart metric every 1 minute
const unsigned long MOTION_DEBOUNCE = 50;       // 50ms debounce for near-instant detection
const unsigned long DETECTION_WINDOW_MS = 60000; // 1 minute window
const unsigned long TIME_SYNC_RETRY_MS = 30000; // Retry NTP sync every 30 seconds
const unsigned long BLYNK_RECONNECT_INTERVAL_MS = 5000; // Retry Blynk connect every 5 seconds

const char* DEVICE_LOCATION = "Phòng khách";

const int MAX_MOTION_EVENTS = 30;
unsigned long motionEventTimes[MAX_MOTION_EVENTS];
int motionEventIndex = 0;
int motionEventCount = 0;

const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
const char* NTP_SERVER_3 = "time.nist.gov";
const long GMT_OFFSET_SEC = 7 * 3600;  // UTC+7 (Vietnam)
const int DAYLIGHT_OFFSET_SEC = 0;
unsigned long lastTimeSyncAttempt = 0;

WiFiClientSecure secureClient;
UniversalTelegramBot bot(BOT_TOKEN, secureClient);

// ========== Function Declarations ==========
void handleMotionDetection();
void sendTelegramAlert(String message);
void updateBlynkDashboard();
void publishAlertMetrics();
void toggleSystem();
void blinkLED(int times);
void initTime();
bool isTimeSynced();
String getTimeString();
String getTelegramTimeString();
void recordMotionEvent(unsigned long eventTime);
int countMotionInWindow(unsigned long currentTime);
String getAlertLevel(int detectionsInWindow);
void logEvent(String event);

// ========== Setup ==========
void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for serial to initialize
  
  Serial.println("\n\n=== ESP32 Motion Alarm System ===");
  
  // Pin Setup
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(LED_ALERT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(LED_ALERT_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Welcome blink
  blinkLED(3);
  
  // WiFi Connection
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize SNTP to get real date/time for alerts
    initTime();
    
    // Set secure client
    secureClient.setInsecure();
    
    // Blynk Setup
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    delay(1000);
    
    if (Blynk.connected()) {
      Serial.println("✓ Blynk Connected!");
      Blynk.logEvent("system_start", "System Started");
    }
  } else {
    Serial.println("\n✗ WiFi Failed!");
  }
  
  Serial.println("System Ready!");
  logEvent("System initialized");
}

// ========== Main Loop ==========
void loop() {
  // Check motion first to minimize detection latency.
  handleMotionDetection();

  // Maintain WiFi and Blynk connections
  if (WiFi.status() == WL_CONNECTED) {
    if (!Blynk.connected()) {
      if (millis() - lastBlynkReconnectAttempt >= BLYNK_RECONNECT_INTERVAL_MS) {
        lastBlynkReconnectAttempt = millis();
        Serial.println("Reconnecting Blynk...");
        Blynk.connect(1000);
      }
    } else {
      Blynk.run();
    }

    // Retry NTP sync periodically if time is still not valid.
    if (!isTimeSynced() && (millis() - lastTimeSyncAttempt >= TIME_SYNC_RETRY_MS)) {
      initTime();
    }

    // Write count-in-window metric to Blynk once per minute.
    if (Blynk.connected() && (millis() - lastMetricWriteTime >= METRIC_WRITE_INTERVAL_MS)) {
      publishAlertMetrics();
      lastMetricWriteTime = millis();
    }
  }
  
  // Update Blynk dashboard
  if (millis() - lastBlynkUpdateTime >= BLYNK_INTERVAL) {
    updateBlynkDashboard();
    lastBlynkUpdateTime = millis();
  }
  
  delay(1);
}

// ========== Motion Detection Handler ==========
void handleMotionDetection() {
  int pirValue = digitalRead(PIR_SENSOR_PIN);
  unsigned long currentTime = millis();
  
  // Motion Detected
  if (pirValue == HIGH && !motionDetected && (currentTime - lastMotionTime > MOTION_DEBOUNCE)) {
    motionDetected = true;
    lastMotionTime = currentTime;
    
    if (systemArmed) {
      Serial.println("\n🚨 MOTION DETECTED! 🚨");

      // Prepare and send Telegram alert immediately when motion is detected.
      alertCount++;
      recordMotionEvent(currentTime);
      int detectionsInLastMinute = countMotionInWindow(currentTime);
      String alertLevel = getAlertLevel(detectionsInLastMinute);

      String alertMsg = "🚨 CẢNH BÁO CHUYỂN ĐỘNG\n";
      alertMsg += "📍 Vị trí: " + String(DEVICE_LOCATION) + "\n";
      alertMsg += "🕒 Giờ phát hiện: " + getTelegramTimeString() + "\n";
      alertMsg += "🔁 Số lần phát hiện: " + String(detectionsInLastMinute) + " (trong 1 phút)\n";
      alertMsg += "🎯 Mức độ: " + alertLevel + "\n";
      alertMsg += "📡 IP thiết bị: " + WiFi.localIP().toString();

      sendTelegramAlert(alertMsg);
      lastTelegramTime = currentTime;
      
      // Visual Alert
      digitalWrite(LED_ALERT_PIN, HIGH);
      blinkLED(5);
      
      // Audio Alert (if buzzer available)
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
      
      // Update Blynk
      Blynk.virtualWrite(V0, 1);  // Motion Status ON
      Blynk.virtualWrite(V1, alertCount);  // Total alert count (instant update)
      Blynk.virtualWrite(V3, getTimeString());  // Last Alert Time
      Blynk.logEvent("motion_alert", "Motion Detected!");
    } else {
      Serial.println("Motion detected but system is disabled");
    }
  }
  
  // No Motion
  else if (pirValue == LOW && motionDetected && (currentTime - lastMotionTime > MOTION_DEBOUNCE)) {
    motionDetected = false;
    lastMotionTime = currentTime;
    
    Serial.println("✓ Motion cleared");
    digitalWrite(LED_ALERT_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Update Blynk
    Blynk.virtualWrite(V0, 0);  // Motion Status OFF
  }
}

// ========== Send Telegram Alert ==========
void sendTelegramAlert(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("--- Telegram payload ---");
    Serial.println(message);
    Serial.println("------------------------");
    bot.sendMessage(CHAT_ID, message, "");
    Serial.println("✓ Telegram alert sent");
  } else {
    Serial.println("✗ WiFi not connected, cannot send Telegram");
  }
}

// ========== Update Blynk Dashboard ==========
void updateBlynkDashboard() {
  if (Blynk.connected()) {
    // V0: Motion Status (already updated in motion detection)
    // V1: Total Alert Count (updated on each motion event)
    // V6: Motion count in the last minute (updated every minute)
    Blynk.virtualWrite(V2, systemArmed ? 1 : 0);  // System Armed Status
    Blynk.virtualWrite(V4, digitalRead(PIR_SENSOR_PIN));  // PIR Raw Value
  }
}

void publishAlertMetrics() {
  if (!Blynk.connected()) {
    return;
  }

  int detectionsInLastMinute = countMotionInWindow(millis());
  Blynk.virtualWrite(V6, detectionsInLastMinute);
}

// ========== Blynk Virtual Pin Handlers ==========

// V2: System Armed/Disarmed Switch
BLYNK_WRITE(V2) {
  systemArmed = param.asInt();
  if (systemArmed) {
    Serial.println("🟢 System Armed");
    Blynk.logEvent("system_armed", "System Armed");
    blinkLED(2);
  } else {
    Serial.println("🔴 System Disarmed");
    Blynk.logEvent("system_disarmed", "System Disarmed");
    digitalWrite(LED_ALERT_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// V5: Reset Alert Count
BLYNK_WRITE(V5) {
  if (param.asInt() == 1) {
    alertCount = 0;
    motionEventCount = 0;
    motionEventIndex = 0;
    Blynk.virtualWrite(V1, alertCount);
    Blynk.virtualWrite(V6, 0);
    Serial.println("Alert count reset");
    Blynk.logEvent("alert_reset", "Alert Count Reset");
  }
}

// ========== Utility Functions ==========

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_ALERT_PIN, HIGH);
    delay(200);
    digitalWrite(LED_ALERT_PIN, LOW);
    delay(200);
  }
}

void initTime() {
  lastTimeSyncAttempt = millis();
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);

  struct tm timeinfo;
  Serial.print("Syncing time");
  for (int i = 0; i < 20; i++) {
    if (getLocalTime(&timeinfo, 1000) && timeinfo.tm_year >= (2020 - 1900)) {
      Serial.println("\n✓ Time synchronized");
      Serial.print("Current time: ");
      Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
      return;
    }
    Serial.print(".");
    delay(200);
  }

  Serial.println("\n⚠ Time sync failed, using uptime fallback");
}

bool isTimeSynced() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  return (timeinfo != nullptr && timeinfo->tm_year >= (2020 - 1900));
}

String getTimeString() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // If SNTP has not synced yet, avoid returning 1970-based timestamps.
  if (!isTimeSynced() || timeinfo == nullptr) {
    return String("Uptime ") + String(millis() / 1000) + String("s");
  }

  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

String getTelegramTimeString() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  if (!isTimeSynced() || timeinfo == nullptr) {
    return String("Uptime ") + String(millis() / 1000) + String("s");
  }

  char buffer[80];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y", timeinfo);
  return String(buffer);
}

void recordMotionEvent(unsigned long eventTime) {
  motionEventTimes[motionEventIndex] = eventTime;
  motionEventIndex = (motionEventIndex + 1) % MAX_MOTION_EVENTS;
  if (motionEventCount < MAX_MOTION_EVENTS) {
    motionEventCount++;
  }
}

int countMotionInWindow(unsigned long currentTime) {
  int count = 0;
  for (int i = 0; i < motionEventCount; i++) {
    if (currentTime - motionEventTimes[i] <= DETECTION_WINDOW_MS) {
      count++;
    }
  }
  return count;
}

String getAlertLevel(int detectionsInWindow) {
  if (detectionsInWindow >= 3) {
    return "Cao";
  }
  if (detectionsInWindow == 2) {
    return "Trung bình";
  }
  return "Thấp";
}

void logEvent(String event) {
  Serial.println("[LOG] " + event);
}

// ========== Blynk Connection Handler ==========
BLYNK_CONNECTED() {
  Serial.println("✓ Blynk reconnected!");
  Blynk.syncVirtual(V2);  // Sync System Armed status
  Blynk.virtualWrite(V1, alertCount);
  publishAlertMetrics();
  lastMetricWriteTime = millis();
  Blynk.logEvent("system_reconnect", "System Reconnected");
}