#define BLYNK_TEMPLATE_ID "TMPL6mcSAdOVC"
#define BLYNK_TEMPLATE_NAME "ESP8266 BLYNK TELEGRAM"
#define BLYNK_AUTH_TOKEN "Za4JrX_t85HA-Iu7JmyC-fMas7kf5_3_"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <TM1637Display.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ===== THÔNG TIN TELEGRAM =====
#define BOTtoken "8621124778:AAGkuf8ptUKbxWczkcp4zHpeqPIbhkmGijg"
#define CHAT_ID "-1003885166476"

// ===== WIFI =====
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ===== PIN CONFIG =====
#define DHTPIN 16
#define DHTTYPE DHT22
#define CLK 18
#define DIO 19
#define LED_PIN 21
#define BTN_PIN 23
#define GAS_PIN 34

// ===== KHỞI TẠO ĐỐI TƯỢNG =====
DHT dht(DHTPIN, DHTTYPE);
TM1637Display display(CLK, DIO);
BlynkTimer timer;
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

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

void updateDisplay()
{
  if (!counterActive)
  {
    display.clear();
    return;
  }
  if (displayMode == 0)
    display.showNumberDec((int)temp);
  else if (displayMode == 1)
    display.showNumberDec(uptimeSeconds % 10000);
  else
    display.showNumberDec(gasValue);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  dht.begin();
  display.setBrightness(0x0f);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.syncVirtual(V4);
  updateSystemState();

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, []()
                    { if(counterActive) uptimeSeconds++; Blynk.virtualWrite(V0, uptimeSeconds); });
  timer.setInterval(3000L, []()
                    { displayMode = (displayMode + 1) % 3; });
  timer.setInterval(500L, updateDisplay);

  bot.sendMessage(CHAT_ID, "🤖 Hệ thống đã khởi động và sẵn sàng nhận lệnh = /start!", "");
}

void loop()
{
  Blynk.run();
  timer.run();

  // Kiểm tra nút nhấn vật lý
  static unsigned long lastBtnTime = 0;
  static int lastBtnValue = HIGH;
  if (millis() - lastBtnTime > 50)
  {
    int v = digitalRead(BTN_PIN);
    if (v != lastBtnValue)
    {
      lastBtnTime = millis();
      lastBtnValue = v;
      if (v == LOW)
      {
        counterActive = !counterActive;
        updateSystemState();
        bot.sendMessage(CHAT_ID, "Nút nhấn vật lý đã thay đổi trạng thái: " + String(counterActive ? "BẬT" : "TẮT"), "");
      }
    }
  }

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