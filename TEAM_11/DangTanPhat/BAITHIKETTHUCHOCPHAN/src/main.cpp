#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include "secrets.h"
// ====================== OLED ======================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====================== WIFI ======================
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// ====================== NTP =======================
// GMT+7 cho Việt Nam
const long  GMT_OFFSET_SEC = 7 * 3600;
const int   DAYLIGHT_OFFSET_SEC = 0;

// Có thể đổi sang pool khác nếu muốn
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.nist.gov";

// ==================== TELEGRAM ====================
String BOT_TOKEN_STR = BOT_TOKEN;
String CHAT_ID_STR   = CHAT_ID;

// ===================== LỊCH =======================
// Đặt 3 mốc giờ mẫu để chụp ảnh minh họa
// Bạn có thể sửa lại cho phù hợp
struct AlarmTime {
  int hour;
  int minute;
  const char* message;
};

AlarmTime alarms[] = {
  {7,  0,  "Thong bao lich 07:00 - bat dau ngay moi"},
  {12, 0,  "Thong bao lich 12:00 - nghi trua"},
  {18, 0,  "Thong bao lich 18:00 - ket thuc ngay hoc tap"}
};

const int ALARM_COUNT = sizeof(alarms) / sizeof(alarms[0]);

// Để tránh gửi lặp lại nhiều lần trong cùng 1 phút
int lastSentDay = -1;
int lastSentMonth = -1;
int lastSentYear = -1;
int lastSentHour = -1;
int lastSentMinute = -1;

// =================== HÀM HIỂN THỊ =================
void showBootScreen(const String& line1, const String& line2 = "", const String& line3 = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ESP32 RTC with NTP");

  display.setCursor(0, 18);
  display.println(line1);

  if (line2.length()) {
    display.setCursor(0, 32);
    display.println(line2);
  }

  if (line3.length()) {
    display.setCursor(0, 46);
    display.println(line3);
  }

  display.display();
}

void showTimeOnOLED(struct tm *timeinfo) {
  char lineDate[20];
  char lineTime[20];
  char lineDow[20];

  strftime(lineDate, sizeof(lineDate), "%d/%m/%Y", timeinfo);
  strftime(lineTime, sizeof(lineTime), "%H:%M:%S", timeinfo);
  strftime(lineDow, sizeof(lineDow), "%A", timeinfo);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Dong ho NTP - ESP32");

  display.setTextSize(2);
  display.setCursor(8, 18);
  display.println(lineTime);

  display.setTextSize(1);
  display.setCursor(20, 46);
  display.println(lineDate);

  display.setCursor(30, 56);
  display.println(lineDow);

  display.display();
}

// ================ TELEGRAM SEND ===================
bool sendTelegramMessage(const String& text) {
  WiFiClientSecure client;
  client.setInsecure(); // Dung cho demo/Wokwi

  HTTPClient https;

String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage";
String payload = "chat_id=" + String(CHAT_ID) + "&text=" + text;
  https.begin(client, url);
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = https.POST(payload);
  String response = https.getString();

  Serial.println("------ TELEGRAM ------");
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  Serial.println("Response:");
  Serial.println(response);
  Serial.println("----------------------");

  https.end();

  return (httpCode > 0 && httpCode == 200);
}

// =============== KIỂM TRA ĐỒNG BỘ GIỜ =============
bool waitForNTPTime() {
  struct tm timeinfo;
  int retry = 0;
  const int maxRetry = 30;

  while (!getLocalTime(&timeinfo) && retry < maxRetry) {
    Serial.println("Dang doi dong bo NTP...");
    delay(1000);
    retry++;
  }

  return retry < maxRetry;
}

// ==================== WIFI CONNECT =================
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  showBootScreen("Dang ket noi WiFi...");
  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi da ket noi!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  showBootScreen("WiFi da ket noi", WiFi.localIP().toString());
  delay(1500);
}

// ================== CHECK ALARM ====================
void checkAndSendScheduledMessage(struct tm *timeinfo) {
  int yearNow   = timeinfo->tm_year + 1900;
  int monthNow  = timeinfo->tm_mon + 1;
  int dayNow    = timeinfo->tm_mday;
  int hourNow   = timeinfo->tm_hour;
  int minuteNow = timeinfo->tm_min;

  for (int i = 0; i < ALARM_COUNT; i++) {
    if (hourNow == alarms[i].hour && minuteNow == alarms[i].minute) {
      // Nếu đã gửi trong chính phút này thì bỏ qua
      if (lastSentYear == yearNow &&
          lastSentMonth == monthNow &&
          lastSentDay == dayNow &&
          lastSentHour == hourNow &&
          lastSentMinute == minuteNow) {
        return;
      }

      char currentTime[30];
      strftime(currentTime, sizeof(currentTime), "%d/%m/%Y %H:%M:%S", timeinfo);

      String msg = "ESP32 NTP Clock\n";
      msg += "Thoi gian hien tai: ";
      msg += currentTime;
      msg += "\n";
      msg += alarms[i].message;

      bool ok = sendTelegramMessage(msg);

      if (ok) {
        Serial.println("Gui Telegram thanh cong!");
      } else {
        Serial.println("Gui Telegram that bai!");
      }

      lastSentYear = yearNow;
      lastSentMonth = monthNow;
      lastSentDay = dayNow;
      lastSentHour = hourNow;
      lastSentMinute = minuteNow;

      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Da gui Telegram!");
      display.setCursor(0, 18);
      display.println(alarms[i].message);
      display.setCursor(0, 38);
      display.println(currentTime);
      display.display();

      delay(3000);
    }
  }
}

// ======================= SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Khong tim thay OLED!");
    while (true) delay(10);
  }

  showBootScreen("Khoi dong he thong...");
  delay(1000);

  connectWiFi();

  showBootScreen("Dang dong bo NTP...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);

  bool ntpOk = waitForNTPTime();
  if (!ntpOk) {
    Serial.println("Dong bo NTP that bai!");
    showBootScreen("Dong bo NTP that bai!");
    while (true) delay(1000);
  }

  Serial.println("Dong bo NTP thanh cong!");
  showBootScreen("Dong bo NTP OK");
  delay(1500);
}

// ======================== LOOP =====================
void loop() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Khong doc duoc thoi gian!");
    showBootScreen("Loi doc thoi gian");
    delay(1000);
    return;
  }

  // In Serial để chụp ảnh minh họa
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  Serial.print("Thoi gian hien tai: ");
  Serial.println(buffer);

  showTimeOnOLED(&timeinfo);
  checkAndSendScheduledMessage(&timeinfo);

  delay(1000);
}