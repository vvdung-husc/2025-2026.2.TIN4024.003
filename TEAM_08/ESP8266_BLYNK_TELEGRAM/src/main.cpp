/*
THÔNG TIN NHÓM 08
1. Trương Hữu Ngọc - Telegram: Ngọc Trương
2. Hồ Bảo Toàn
3. Nguyễn Vỹ Nguyên


*/
#define BLYNK_TEMPLATE_ID "TMPL6mcSAdOVC"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "Za4JrX_t85HA-Iu7JmyC-fMas7kf5_3_"

// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiClientSecure.h>
// #include <BlynkSimpleEsp32.h>
// #include <DHT.h>
// #include <TM1637Display.h>
// #include <UniversalTelegramBot.h>
// #include <ArduinoJson.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>

// ===== THÔNG TIN TELEGRAM =====
#define BOTtoken "8660322756:AAEootdYRngv0BH9YgW4vBsfg1Wb0hEBBXU"
#define CHAT_ID "-5194733008"

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
// #define DHTPIN 16
// #define DHTTYPE DHT22
// #define CLK 18
// #define DIO 19
// #define LED_PIN 4
// #define BTN_PIN 23
// #define GAS_PIN 34

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
#define DHTPIN 14 // D5
#define DHTTYPE DHT22

#define LED_PIN 2 // D4

#define OLED_SDA 4 // D2
#define OLED_SCL 5 // D1

#define BTN_PIN 0 // D3

#define GAS_PIN A0 // Analog

#define CLK 12 // D6
#define DIO 13 // D7

// ===== KHỞI TẠO ĐỐI TƯỢNG =====
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
// Khởi tạo OLED với địa chỉ 0x3C (thường dùng trên Wokwi)
// Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// ===== BIẾN HỆ THỐNG =====
unsigned long uptimeSeconds = 0;
bool counterActive = true;
int displayMode = 0;

float temp = 0, humid = 0;
float lastTemp = 0, lastHumid = 0;
int gasValue = 0;

unsigned long lastTimeBotRan;
int botRequestDelay = 1000; // Kiểm tra tin nhắn mỗi 1 giây

// ===== ĐỒNG BỘ TRẠNG THÁI =====
void updateSystemState()
{
  digitalWrite(LED_PIN, counterActive ? HIGH : LOW);
  Blynk.virtualWrite(V4, counterActive);
}

// ===== XỬ LÝ TIN NHẮN TELEGRAM =====
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Người dùng không xác định", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/led_on")
    {
      counterActive = true;
      updateSystemState();
      bot.sendMessage(CHAT_ID, "✅ LED đã được BẬT bởi " + from_name, "");
    }
    else if (text == "/led_off")
    {
      counterActive = false;
      updateSystemState();
      bot.sendMessage(CHAT_ID, "❌ LED đã được TẮT bởi " + from_name, "");
    }
    else if (text == "/led_status")
    {
      String status = "💡 Trạng thái LED: " + String(counterActive ? "ĐANG BẬT" : "ĐANG TẮT");
      bot.sendMessage(CHAT_ID, status, "");
    }
    else if (text == "/get_weather")
    {
      String weather = "🌡️ *Thông tin thời tiết hiện tại:*\n";
      weather += "Nhiệt độ: " + String(temp, 1) + "°C\n";
      weather += "Độ ẩm: " + String(humid, 1) + "%";
      bot.sendMessage(CHAT_ID, weather, "Markdown");
    }
    else if (text == "/start")
    {
      String welcome = "Chào mừng " + from_name + ".\n";
      welcome += "Các lệnh hỗ trợ:\n";
      welcome += "/led_on : Bật LED\n";
      welcome += "/led_off : Tắt LED\n";
      welcome += "/led_status : Kiểm tra trạng thái LED\n";
      welcome += "/get_weather : Xem nhiệt độ, độ ẩm";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
  }
}

// ===== ĐỌC CẢM BIẾN =====
void sendSensorData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  gasValue = analogRead(GAS_PIN);

  if (!isnan(h) && !isnan(t))
  {
    temp = t;
    humid = h;

    Blynk.virtualWrite(V1, temp);
    Blynk.virtualWrite(V2, humid);
    Blynk.virtualWrite(V3, gasValue);

    // Tự động báo cáo nếu có thay đổi lớn
    if (abs(temp - lastTemp) >= 0.5 || abs(humid - lastHumid) >= 2.0)
    {
      String msg = "📊 *Thay đổi môi trường:*\nTemp: " + String(temp, 1) + "°C | Hum: " + String(humid, 1) + "%";
      bot.sendMessage(CHAT_ID, msg, "Markdown");
      lastTemp = temp;
      lastHumid = humid;
    }

    if (gasValue > 2500)
    {
      bot.sendMessage(CHAT_ID, "⚠️ *CẢNH BÁO:* Phát hiện nồng độ Gas cao (" + String(gasValue) + ")!", "Markdown");
    }
  }
}

// void updateDisplay()
// {
//   // 1. TM1637 (Uptime)
//   if (counterActive)
//   {
//     display.showNumberDec(uptimeSeconds % 10000);
//   }
//   else
//   {
//     display.clear();
//   }

//   // 2. OLED
//   oled.clearDisplay();
//   oled.setTextSize(1);
//   oled.setTextColor(SSD1306_WHITE);

//   // Nếu pause thì hiển thị trạng thái
//   if (!counterActive)
//   {
//     oled.setCursor(0, 0);
//     oled.println("--- PAUSED ---");
//   }
//   else
//   {
//     oled.setCursor(0, 0);
//     oled.println("--- MONITORING ---");
//   }

//   // Hiển thị Nhiệt độ
//   oled.setCursor(0, 20);
//   oled.print("Temp:  ");
//   oled.print(temp, 1);
//   oled.println(" C");

//   // Hiển thị Độ ẩm
//   oled.setCursor(0, 35);
//   oled.print("Humid: ");
//   oled.print(humid, 1);
//   oled.println(" %");

//   // Hiển thị Gas
//   oled.setCursor(0, 50);
//   oled.print("Gas:   ");
//   oled.print(gasValue);

//   // Thanh bar Gas
//   int barWidth = map(gasValue, 0, 4095, 0, 50);
//   oled.drawRect(70, 50, 52, 10, SSD1306_WHITE);
//   oled.fillRect(71, 51, barWidth, 8, SSD1306_WHITE);

//   oled.display();
// }
void updateDisplay()
{
  // ===== TM1637 =====
  if (counterActive)
    display.showNumberDec(uptimeSeconds % 10000);
  else
    display.clear();

  // ===== OLED (U8g2) =====
  oled.clearBuffer();

  if (!counterActive)
    oled.drawStr(0, 10, "--- PAUSED ---");
  else
    oled.drawStr(0, 10, "--- MONITOR ---");

  char buf[30];

  sprintf(buf, "Temp: %.1f C", temp);
  oled.drawStr(0, 25, buf);

  sprintf(buf, "Hum : %.1f %%", humid);
  oled.drawStr(0, 40, buf);

  sprintf(buf, "Gas : %d", gasValue);
  oled.drawStr(0, 55, buf);

  // Thanh bar gas (ESP8266 max 1023)
  int barWidth = map(gasValue, 0, 1023, 0, 50);
  oled.drawFrame(70, 45, 52, 10);
  oled.drawBox(71, 46, barWidth, 8);

  oled.sendBuffer();
}

// void setup()
// {
//   Serial.begin(115200);
//   pinMode(LED_PIN, OUTPUT);
//   pinMode(BTN_PIN, INPUT_PULLUP);

//   Wire.begin(21, 22);
//   dht.begin();
//   display.setBrightness(0x0f);
//   client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

//   Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
//   Blynk.syncVirtual(V4);
//   updateSystemState();

//   // Khởi tạo OLED
//   if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
//   {
//     Serial.println(F("SSD1306 allocation failed"));
//   }
//   oled.clearDisplay();
//   oled.display();

//   // Giữ nguyên các timer cũ
//   timer.setInterval(2000L, sendSensorData);
//   timer.setInterval(1000L, []()
//                     {
//     if(counterActive) uptimeSeconds++;
//     Blynk.virtualWrite(V0, uptimeSeconds); });
//   timer.setInterval(500L, updateDisplay); // Cập nhật cả 2 màn hình mỗi 0.5s

//   bot.sendMessage(CHAT_ID, "🤖 Hệ thống đã khởi động và sẵn sàng nhận lệnh = /start!", "");
// }
void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  Wire.begin(OLED_SDA, OLED_SCL); // ✅ sửa lại
  dht.begin();
  display.setBrightness(0x0f);

  client.setInsecure(); // ✅ sửa cho ESP8266

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.syncVirtual(V4);
  updateSystemState();

  // OLED
  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(0, 20, "System Starting...");
  oled.sendBuffer();

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, []()
                    {
    if(counterActive) uptimeSeconds++; 
    Blynk.virtualWrite(V0, uptimeSeconds); });

  timer.setInterval(500L, updateDisplay);

  bot.sendMessage(CHAT_ID, "🤖 He thong da khoi dong!", "");
}

void loop()
{
  Blynk.run();
  timer.run();

  // -------- BUTTON toggle system --------
  static bool lastBtn = HIGH;
  bool btn = digitalRead(BTN_PIN);

  // LED phản ánh trạng thái (giống LED_BLUE trong ví dụ)
  digitalWrite(LED_PIN, counterActive ? HIGH : LOW);

  if (lastBtn == HIGH && btn == LOW)
  {
    counterActive = !counterActive; // toggle trạng thái hệ thống
    updateSystemState();

    // Gửi Telegram
    bot.sendMessage(CHAT_ID,
                    "Nút nhấn vật lý: " + String(counterActive ? "BẬT" : "TẮT"), "");

    // Điều khiển TM1637 giống logic mẫu
    if (!counterActive)
      display.clear();
    else
      display.showNumberDec(uptimeSeconds % 10000, true);

    delay(200); // chống rung đơn giản
  }

  lastBtn = btn;

  // Kiểm tra tin nhắn Telegram
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

BLYNK_WRITE(V4)
{
  counterActive = param.asInt();
  updateSystemState();
}