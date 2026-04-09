#define BLYNK_TEMPLATE_ID "TMPL6ZT4QsCKz"
#define BLYNK_TEMPLATE_NAME "MPU6050ESP32"
#define BLYNK_AUTH_TOKEN "AA7VaMD5sssoPbnUjcVdWThJ9uErHCPE"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// --- CẤU HÌNH TELEGRAM ---
#define BOTtoken "8642055361:AAGRRrMZBThulXx-ojrFISwJiPYQONm0zwI"
#define CHAT_ID "-1003743005466" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define LED_PIN 2
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

Adafruit_MPU6050 mpu;
BlynkTimer timer;

// Biến toàn cục để lưu trữ dữ liệu mới nhất cho Telegram Bot
float g_ax, g_ay, g_az, g_gx, g_gy, g_gz, g_temp;
float last_mag = 1.0; 
const float THRESHOLD = 0.25; 
unsigned long alarmStartTime = 0;
bool isAlarming = false;
const long ALARM_DURATION = 5000;
unsigned long lastTelegramTime = 0;
const long TELEGRAM_COOLDOWN = 10000; 

void handleLED() {
  if (isAlarming) {
    if (millis() - alarmStartTime < ALARM_DURATION) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); 
    } else {
      isAlarming = false;         
      digitalWrite(LED_PIN, LOW); 
    }
  } else {
    digitalWrite(LED_PIN, LOW);   
  }
}

// Hàm bổ sung: Xử lý tin nhắn đến từ Telegram
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) continue; // Chỉ trả lời tin nhắn từ CHAT_ID đã cấu hình

    String text = bot.messages[i].text;
    if (text == "/trangthai" || text == "/start") {
      String statusMsg = "*TRẠNG THÁI HỆ THỐNG HIỆN TẠI*\n";
      statusMsg += "----------------------------\n";
      statusMsg += "Gia tốc: " + String(g_ax, 2) + "g, " + String(g_ay, 2) + "g, " + String(g_az, 2) + "g\n";
      statusMsg += "Vòng quay: " + String(g_gx, 0) + ", " + String(g_gy, 0) + ", " + String(g_gz, 0) + "\n";
      statusMsg += "Nhiệt độ: " + String(g_temp, 1) + "°C\n";
      statusMsg += "Trạng thái: " + String(isAlarming ? "NGUY HIỂM" : " AN TOÀN") + "\n";
      statusMsg += "----------------------------\n";
      
      bot.sendMessage(chat_id, statusMsg, "Markdown");
    }
  }
}

// Kiểm tra tin nhắn Telegram định kỳ
void checkTelegram() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while(numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}

void sendTelegramMessage(float ax, float ay, float az, float gx, float gy, float gz, float temp) {
  String msg = "*CẢNH BÁO RUNG ĐỘNG!*\n";
  msg += "------------------------\n";
  msg += "Gia tốc: " + String(ax, 2) + "g, " + String(ay, 2) + "g, " + String(az, 2) + "g\n";
  msg += "Vòng quay: " + String(gx, 0) + ", " + String(gy, 0) + ", " + String(gz, 0) + "\n";
  msg += "Nhiệt độ: " + String(temp, 1) + "°C\n";
  msg += "------------------------\n";
  msg += "*Trạng thái: NGUY HIỂM*";
  
  bot.sendMessage(CHAT_ID, msg, "Markdown");
}

void updateSystem() {
  sensors_event_t a, g, temp;
  if (!mpu.getEvent(&a, &g, &temp)) return;

  // Cập nhật vào biến toàn cục
  g_ax = a.acceleration.x / 9.806;
  g_ay = a.acceleration.y / 9.806;
  g_az = a.acceleration.z / 9.806;
  g_gx = g.gyro.x * 57.295;
  g_gy = g.gyro.y * 57.295;
  g_gz = g.gyro.z * 57.295;
  g_temp = temp.temperature;

  // Gửi Blynk
  Blynk.virtualWrite(V0, g_ax); Blynk.virtualWrite(V1, g_ay); Blynk.virtualWrite(V2, g_az);
  Blynk.virtualWrite(V3, g_gx); Blynk.virtualWrite(V4, g_gy); Blynk.virtualWrite(V5, g_gz);
  Blynk.virtualWrite(V6, g_temp);

  float current_mag = sqrt(pow(g_ax, 2) + pow(g_ay, 2) + pow(g_az, 2));
  float delta = abs(current_mag - last_mag);

  if (delta > THRESHOLD) {
    if (!isAlarming) {
      isAlarming = true;
      alarmStartTime = millis();
      Blynk.logEvent("canh_bao_rung", "NGUY HIEM!");
      
      if (millis() - lastTelegramTime > TELEGRAM_COOLDOWN) {
        sendTelegramMessage(g_ax, g_ay, g_az, g_gx, g_gy, g_gz, g_temp);
        lastTelegramTime = millis();
      }
    }
    Blynk.virtualWrite(V7, "NGUY HIEM: PHAT HIEN RUNG!");
  } else if (!isAlarming) {
    Blynk.virtualWrite(V7, "He thong: AN TOAN");
  }

  // Cập nhật OLED
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Rung: "); display.print(isAlarming ? "!! CANH BAO !!" : "Binh thuong");
  display.setCursor(0, 18); display.printf("A:%.2f,%.2f,%.2f", g_ax, g_ay, g_az);
  display.setCursor(0, 36); display.printf("R:%.0f,%.0f,%.0f", g_gx, g_gy, g_gz);
  display.setCursor(0, 54); display.printf("Temp: %.1f C", g_temp);
  display.display();

  last_mag = current_mag;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  client.setInsecure(); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED fail");
  if (!mpu.begin()) { Serial.println("MPU6050 fail"); while (1) yield(); }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(500L, updateSystem);
  timer.setInterval(100L, handleLED);
  timer.setInterval(1000L, checkTelegram); // Kiểm tra tin nhắn mỗi 1 giây
}

void loop() {
  Blynk.run();
  timer.run();
}