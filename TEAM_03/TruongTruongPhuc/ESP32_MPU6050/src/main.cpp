#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <time.h>

// Blynk cần các macro này được định nghĩa trước khi include header Blynk.
#define BLYNK_TEMPLATE_ID "TMPL6aPpBzFlw"
#define BLYNK_TEMPLATE_NAME "ESP32 MPU6050"
#define BLYNK_AUTH_TOKEN "ctYMNzwkle1erMp_JfgRQqgN7IkttOSh"   // Thay bằng Auth Token của bạn
#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>
#if __has_include(<WiFiClientSecure.h>)
#include <WiFiClientSecure.h>
using TelegramClient = WiFiClientSecure;
#define TELEGRAM_CLIENT_SECURE 1
#else
using TelegramClient = WiFiClient;
#define TELEGRAM_CLIENT_SECURE 0
#endif
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ==================== TELEGRAM CONFIG ====================
#define BOT_TOKEN "8670966008:AAG4MipyrwGNU6VS7LbUx7j-pTdVxyYdndY"
#define CHAT_ID "6251864754"

// ==================== BLYNK CONFIG ====================


// ==================== WIFI CONFIG ====================
const char* ssid = "Wokwi-GUEST";               // Thay bằng SSID WiFi
const char* password = "";       // Thay bằng Password WiFi
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 7 * 3600;   // GMT+7 (Việt Nam)
const int DAYLIGHT_OFFSET_SEC = 0;

// ==================== VIRTUAL PINS (BLYNK) ====================
#define V_VIBRATION_X V0   // Độ rung trục X (gauge)
#define V_VIBRATION_Y V1   // Độ rung trục Y (gauge)
#define V_VIBRATION_Z V2   // Độ rung trục Z (gauge)
#define V_TOTAL_VIBRATION V3    // Tổng độ rung (gauge)
#define V_CHART_X V4       // Biểu đồ trục X
#define V_CHART_Y V5       // Biểu đồ trục Y
#define V_CHART_Z V6       // Biểu đồ trục Z
#define V_CHART_TOTAL V7   // Biểu đồ tổng độ rung
#define V_STATUS V8        // Trạng thái (LED)
#define V_ALERT_LEVEL V9   // Mức cảnh báo (slider)
#define V_LAST_ALERT V10   // Lần cảnh báo cuối cùng (text label)

// ==================== SENSOR CONFIG ====================
Adafruit_MPU6050 mpu;
const int I2C_SDA = 21;
const int I2C_SCL = 22;
const int LED_PIN = 32;  // GPIO 32 điều khiển LED
// ==================== VIBRATION THRESHOLDS ====================
float VIBRATION_THRESHOLD = 3.0;  // Mức rung (G) - có thể điều chỉnh qua Blynk
const float MAX_THRESHOLD = 10.0;
const float MIN_THRESHOLD = 0.5;

// ==================== TIMING ====================
unsigned long lastSensorRead = 0;
unsigned long lastTelegramAlert = 0;
unsigned long lastBlynkUpdate = 0;
const unsigned long SENSOR_INTERVAL = 100;      // 100ms
const unsigned long TELEGRAM_COOLDOWN = 30000;  // 30 giây (tránh spam)
const unsigned long BLYNK_UPDATE_INTERVAL = 500; // 500ms

// ==================== TELEGRAM BOT ====================
TelegramClient client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long bot_lasttime = 0;
const unsigned long BOT_MTBS = 1000; // Mean time between scans

// ==================== VARIABLES ====================
struct VibrationData {
  float accelX, accelY, accelZ;
  float vibrationX, vibrationY, vibrationZ;
  float totalVibration;
  bool isAlerting;
};

VibrationData currentData = {0, 0, 0, 0, 0, 0, 0, false};
char lastAlertTime[30] = "Never";

// ==================== FUNCTION DECLARATIONS ====================
void setupWiFi();
void setupBlynk();
void setupSensor();
void setupTime();
void formatCurrentTime(char* buffer, size_t bufferSize);
void readSensor();
void checkVibrationAlert();
void sendTelegramAlert(float vibration);
void sendTelegramRecovery(float vibration);
void updateBlynkDisplay();
void handleNewMessages(int numNewMessages);

// ==================== BLYNK VIRTUAL PIN HANDLERS ====================
BLYNK_WRITE(V_ALERT_LEVEL) {
  VIBRATION_THRESHOLD = param.asFloat();
  if (VIBRATION_THRESHOLD > MAX_THRESHOLD) VIBRATION_THRESHOLD = MAX_THRESHOLD;
  if (VIBRATION_THRESHOLD < MIN_THRESHOLD) VIBRATION_THRESHOLD = MIN_THRESHOLD;
  
  Serial.print("Mức cảnh báo mới: ");
  Serial.print(VIBRATION_THRESHOLD);
  Serial.println(" G");
}

BLYNK_CONNECTED() {
  Serial.println("Blynk đã kết nối!");
  Blynk.syncVirtual(V_ALERT_LEVEL);
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);  // Thiết lập LED làm OUTPUT
  digitalWrite(LED_PIN, LOW); // Tắt LED ban đầu
  Serial.println("\n\n==================== VIBRATION MONITORING SYSTEM ====================");
  Serial.println("Khởi tạo hệ thống giám sát độ rung...");
  
  setupSensor();
  setupWiFi();
  setupTime();
  setupBlynk();
  
  Serial.println("Hệ thống sẵn sàng!");
}

// ==================== LOOP ====================
void loop() {
  unsigned long currentTime = millis();
  
  // Đọc cảm biến
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    readSensor();
    checkVibrationAlert();
    lastSensorRead = currentTime;
  }
  
  // Cập nhật Blynk
  if (currentTime - lastBlynkUpdate >= BLYNK_UPDATE_INTERVAL) {
    if (Blynk.connected()) {
      updateBlynkDisplay();
    }
    lastBlynkUpdate = currentTime;
  }
  
  // Kiểm tra tin nhắn Telegram
  if (currentTime - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = currentTime;
  }
  
  Blynk.run();
}

// ==================== SETUP FUNCTIONS ====================
void setupWiFi() {
  Serial.print("Kết nối WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi đã kết nối! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Kết nối WiFi thất bại!");
  }
  
#if TELEGRAM_CLIENT_SECURE
  client.setInsecure(); // Vô hiệu hóa kiểm tra SSL
#endif
}

void setupBlynk() {
  Serial.println("Kết nối Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  
  int timeout = 0;
  while (!Blynk.connected() && timeout < 20) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  
  if (Blynk.connected()) {
    Serial.println();
    Serial.println("Blynk đã kết nối!");
  } else {
    Serial.println();
    Serial.println("Kết nối Blynk thất bại!");
  }
}

void setupTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  Serial.print("Đang đồng bộ thời gian NTP");
  for (int i = 0; i < 20; i++) {
    time_t now = time(nullptr);
    if (now > 100000) {
      Serial.println(" - thành công!");
      return;
    }
    delay(250);
    Serial.print(".");
  }
  Serial.println(" - chưa đồng bộ được, sẽ dùng thời gian uptime tạm thời.");
}

void formatCurrentTime(char* buffer, size_t bufferSize) {
  time_t now = time(nullptr);
  if (now > 100000) {
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
    return;
  }

  unsigned long seconds = millis() / 1000;
  unsigned long hours = seconds / 3600;
  unsigned long minutes = (seconds % 3600) / 60;
  unsigned long secs = seconds % 60;
  snprintf(buffer, bufferSize, "Uptime %02lu:%02lu:%02lu", hours, minutes, secs);
}

void setupSensor() {
  Wire.begin(I2C_SDA, I2C_SCL);
  
  if (!mpu.begin()) {
    Serial.println("Không tìm thấy cảm biến MPU6050!");
    while (1) {
      delay(10);
    }
  }
  
  // Cấu hình cảm biến
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("MPU6050 khởi tạo thành công!");
}

// ==================== SENSOR READING ====================
void readSensor() {
  // Đọc gia tốc từ cảm biến (m/s^2), rồi đổi sang đơn vị g.
  sensors_event_t accelEvent, gyroEvent, tempEvent;
  mpu.getEvent(&accelEvent, &gyroEvent, &tempEvent);

  const float MS2_TO_G = 9.80665f;
  currentData.accelX = accelEvent.acceleration.x / MS2_TO_G;
  currentData.accelY = accelEvent.acceleration.y / MS2_TO_G;
  currentData.accelZ = accelEvent.acceleration.z / MS2_TO_G;
  
  // Tính độ rung (lấy giá trị tuyệt đối)
  currentData.vibrationX = abs(currentData.accelX);
  currentData.vibrationY = abs(currentData.accelY);
  currentData.vibrationZ = abs(currentData.accelZ);
  
  // Tính tổng độ rung (vector magnitude)
  currentData.totalVibration = sqrt(
    currentData.vibrationX * currentData.vibrationX +
    currentData.vibrationY * currentData.vibrationY +
    currentData.vibrationZ * currentData.vibrationZ
  );
  
  // In ra Serial
  Serial.print("X: ");
  Serial.print(currentData.vibrationX, 2);
  Serial.print("G | Y: ");
  Serial.print(currentData.vibrationY, 2);
  Serial.print("G | Z: ");
  Serial.print(currentData.vibrationZ, 2);
  Serial.print("G | TOTAL: ");
  Serial.print(currentData.totalVibration, 2);
  Serial.println("G");
}

// ==================== VIBRATION ALERT ====================
void checkVibrationAlert() {
  unsigned long currentTime = millis();
  
  bool exceedsThreshold = currentData.totalVibration > VIBRATION_THRESHOLD;
  
  if (exceedsThreshold && !currentData.isAlerting) {
    // Vừa vượt ngưỡng
    currentData.isAlerting = true;

    if (lastTelegramAlert == 0 || currentTime - lastTelegramAlert >= TELEGRAM_COOLDOWN) {
      // Cập nhật thời gian trước khi gửi để tránh hiển thị "Never".
      formatCurrentTime(lastAlertTime, sizeof(lastAlertTime));
      sendTelegramAlert(currentData.totalVibration);
      lastTelegramAlert = currentTime;
    }
  } else if (!exceedsThreshold && currentData.isAlerting) {
    // Rung động quay lại bình thường
    currentData.isAlerting = false;
    Serial.println("Rung động quay lại bình thường.");
    sendTelegramRecovery(currentData.totalVibration);
  }
}

void sendTelegramAlert(float vibration) {
  String message = "🚨 <b>CẢNH BÁO RUNG ĐỘNG!</b>\n\n";
  message += "📊 Độ rung: <b>";
  message += String(vibration, 2);
  message += "G</b>\n";
  message += "⚠️ Ngưỡng cảnh báo: <b>";
  message += String(VIBRATION_THRESHOLD, 2);
  message += "G</b>\n";
  message += "📍 Tên thiết bị: Vibration Monitor\n";
  message += "🕐 Thời gian: ";
  message += lastAlertTime;
  
  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, message, "HTML");
    Serial.println("Cảnh báo Telegram đã gửi!");
  } else {
    Serial.println("WiFi chưa kết nối, không thể gửi cảnh báo Telegram!");
  }
}

void sendTelegramRecovery(float vibration) {
  char recoveryTime[30];
  formatCurrentTime(recoveryTime, sizeof(recoveryTime));

  String message = "✅ <b>RUNG ĐỘNG ĐÃ ỔN ĐỊNH</b>\n\n";
  message += "📉 Độ rung hiện tại: <b>";
  message += String(vibration, 2);
  message += "G</b>\n";
  message += "⚠️ Ngưỡng cảnh báo: <b>";
  message += String(VIBRATION_THRESHOLD, 2);
  message += "G</b>\n";
  message += "📍 Tên thiết bị: Vibration Monitor\n";
  message += "🕐 Thời gian: ";
  message += recoveryTime;

  if (WiFi.status() == WL_CONNECTED) {
    bot.sendMessage(CHAT_ID, message, "HTML");
    Serial.println("Thông báo ổn định đã gửi!");
  } else {
    Serial.println("WiFi chưa kết nối, không thể gửi thông báo ổn định!");
  }
}

// ==================== BLYNK UPDATE ====================
void updateBlynkDisplay() {
  // Cập nhật Gauge (Đồng hồ)
  Blynk.virtualWrite(V_VIBRATION_X, currentData.vibrationX);
  Blynk.virtualWrite(V_VIBRATION_Y, currentData.vibrationY);
  Blynk.virtualWrite(V_VIBRATION_Z, currentData.vibrationZ);
  Blynk.virtualWrite(V_TOTAL_VIBRATION, currentData.totalVibration);
  
  // Cập nhật Chart (Biểu đồ)
  Blynk.virtualWrite(V_CHART_X, currentData.vibrationX);
  Blynk.virtualWrite(V_CHART_Y, currentData.vibrationY);
  Blynk.virtualWrite(V_CHART_Z, currentData.vibrationZ);
  Blynk.virtualWrite(V_CHART_TOTAL, currentData.totalVibration);
  
  // Cập nhật trạng thái
  int ledStatus = currentData.isAlerting ? 255 : 0;
  Blynk.virtualWrite(V_STATUS, ledStatus);
  // Cập nhật LED vật lý
  if (currentData.isAlerting) {
    digitalWrite(LED_PIN, HIGH);  // Bật LED khi có cảnh báo
  } else {
    digitalWrite(LED_PIN, LOW);   // Tắt LED khi bình thường
  }
  // Cập nhật thời gian cảnh báo cuối cùng
  Blynk.virtualWrite(V_LAST_ALERT, lastAlertTime);
}

// ==================== TELEGRAM MESSAGE HANDLER ====================
void handleNewMessages(int numNewMessages) {
  Serial.println("Handling new messages: " + String(numNewMessages));
  
  for (int i = 0; i < numNewMessages; i++) {
    telegramMessage msg = bot.messages[i];
    String chat_id = msg.chat_id;
    String text = msg.text;
    String from_name = msg.from_name;
    
    if (text == "/start") {
      String welcome = "Xin chào " + from_name + "!\n";
      welcome += "Tôi là bot giám sát độ rung.\n\n";
      welcome += "Các lệnh có sẵn:\n";
      welcome += "/status - Kiểm tra trạng thái hiện tại\n";
      welcome += "/threshold - Xem ngưỡng cảnh báo\n";
      welcome += "/help - Trợ giúp\n";
      bot.sendMessage(chat_id, welcome);
    }
    
    if (text == "/status") {
      String response = "📊 <b>TRẠNG THÁI HIỆN TẠI</b>\n\n";
      response += "Độ rung trục X: <b>" + String(currentData.vibrationX, 2) + " G</b>\n";
      response += "Độ rung trục Y: <b>" + String(currentData.vibrationY, 2) + " G</b>\n";
      response += "Độ rung trục Z: <b>" + String(currentData.vibrationZ, 2) + " G</b>\n";
      response += "Tổng độ rung: <b>" + String(currentData.totalVibration, 2) + " G</b>\n";
      response += "Ngưỡng cảnh báo: <b>" + String(VIBRATION_THRESHOLD, 2) + " G</b>\n";
      response += "Trạng thái: " + String(currentData.isAlerting ? "🔴 <b>CÓ CẢNH BÁO</b>" : "🟢 <b>BÌNH THƯỜNG</b>") + "\n";
      bot.sendMessage(chat_id, response, "HTML");
    }
    
    if (text == "/threshold") {
      String response = "Ngưỡng cảnh báo hiện tại: <b>" + String(VIBRATION_THRESHOLD, 2) + " G</b>\n";
      response += "Để thay đổi, vui lòng sử dụng ứng dụng Blynk.";
      bot.sendMessage(chat_id, response, "HTML");
    }
    
    if (text == "/help") {
      String help = "📖 <b>TRỢ GIÚP</b>\n\n";
      help += "/start - Hiển thị menu chính\n";
      help += "/status - Xem tình trạng rung động hiện tại\n";
      help += "/threshold - Xem ngưỡng cảnh báo\n";
      help += "/help - Hiển thị trợ giúp này\n\n";
      help += "💡 Mẹo: Sử dụng Blynk để điều chỉnh các cài đặt!";
      bot.sendMessage(chat_id, help, "HTML");
    }
  }
}