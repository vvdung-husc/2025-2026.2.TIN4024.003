// =============================================
//   IoT - Team X   (Blynk + Telegram + OLED)
// =============================================

#define BLYNK_TEMPLATE_ID "TMPL6AJWRTv7y"
#define BLYNK_TEMPLATE_NAME "Blynk Traffic DHT sensor"
#define BLYNK_AUTH_TOKEN "EZPKS73v1Rey8uiWh1M34pFWEIMu4Thz"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// ── WiFi ───────────────────────────────────────
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

// ── Blynk & Timer ──────────────────────────────
BlynkTimer timer;

// ── OLED SSD1306 (thay TM1637) ─────────────────
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C // Thường là 0x3C hoặc 0x3D, kiểm tra bằng I2C scanner nếu cần

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Pins ───────────────────────────────────────
#define DHTPIN 16
#define DHTTYPE DHT22
#define LED_PIN 21
#define BTN_PIN 23
#define MQ2_PIN 34 // Analog pin cho MQ2 (nếu có thật)

// ── Telegram ───────────────────────────────────
#define BOTtoken "8660322756:AAEootdYRngv0BH9YgW4vBsfg1Wb0hEBBXU"
#define CHAT_ID "-1003885166476" // Group ID của nhóm "IoT - Team X"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan = 0;
const int bot_delay = 1000;

// ── Biến hệ thống ──────────────────────────────
unsigned long uptimeSeconds = 0;
bool ledState = false; // Trạng thái LED (đồng bộ Blynk ↔ Telegram)
bool showTemperature = true;

float lastTemp = -999; // Để phát hiện thay đổi gửi Telegram
float lastHum = -999;
int lastGas = -1;

// Giá trị ngưỡng thay đổi để gửi thông báo Telegram (đơn vị: °C, %, ppm)
const float TEMP_CHANGE_THRESHOLD = 0.8;
const float HUM_CHANGE_THRESHOLD = 3.0;
const int GAS_CHANGE_THRESHOLD = 15;

// ── Đối tượng cảm biến ─────────────────────────
DHT dht(DHTPIN, DHTTYPE);

// ── Chống dội nút ──────────────────────────────
bool IsReady(unsigned long &ulTimer, uint32_t ms)
{
  unsigned long now = millis();
  if (now - ulTimer < ms)
    return false;
  ulTimer = now;
  return true;
}

// ── Cập nhật LED phần cứng & Blynk ─────────────
void updateLedState()
{
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  Blynk.virtualWrite(V4, ledState ? 1 : 0);
}

// ── Xử lý nút bấm vật lý ───────────────────────
void updateButton()
{
  static unsigned long lastDebounce = 0;
  static int lastBtnState = HIGH;

  if (!IsReady(lastDebounce, 50))
    return;

  int reading = digitalRead(BTN_PIN);
  if (reading == lastBtnState)
    return;
  lastBtnState = reading;

  if (reading == LOW)
  { // Nhấn nút (active LOW)
    ledState = !ledState;
    Serial.println(ledState ? "Button → LED ON" : "Button → LED OFF");
    updateLedState();

    // Gửi thông báo Telegram khi nhấn nút vật lý (tuỳ chọn)
    // bot.sendMessage(CHAT_ID, ledState ? "LED bật từ nút vật lý" : "LED tắt từ nút vật lý", "");
  }
}

// ── Đọc cảm biến & gửi Blynk + kiểm tra Telegram ────────
void sendSensorData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("DHT lỗi!");
    return;
  }

  // Gas: nếu không gắn thật → random 300–800 (ppm giả lập)
  int gasValue;
#ifdef WOKWI // Nếu chạy trên Wokwi thì luôn random
  gasValue = random(320, 780);
#else
  int raw = analogRead(MQ2_PIN);
  gasValue = map(raw, 0, 4095, 200, 10000); // Giả lập mapping, chỉnh lại nếu có calib thật
#endif

  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V3, gasValue); // Thêm V3 cho gas (tạo widget Gauge/Chart trên Blynk)

  // Kiểm tra thay đổi để gửi Telegram
  bool shouldNotify = false;
  String msg = "📊 Cập nhật từ Team X:\n";

  if (abs(t - lastTemp) >= TEMP_CHANGE_THRESHOLD)
  {
    msg += "Nhiệt độ: " + String(t, 1) + " °C\n";
    lastTemp = t;
    shouldNotify = true;
  }
  if (abs(h - lastHum) >= HUM_CHANGE_THRESHOLD)
  {
    msg += "Độ ẩm: " + String(h, 1) + " %\n";
    lastHum = h;
    shouldNotify = true;
  }
  if (abs(gasValue - lastGas) >= GAS_CHANGE_THRESHOLD)
  {
    msg += "Khí gas: " + String(gasValue) + " ppm\n";
    lastGas = gasValue;
    shouldNotify = true;
  }

  if (shouldNotify)
  {
    bot.sendMessage(CHAT_ID, msg, "");
  }

  uptimeSeconds++;
  Blynk.virtualWrite(V0, uptimeSeconds);
}

// ── Hiển thị OLED luân phiên ───────────────────
void updateOLED()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("IoT - Team X");

  if (ledState)
  {
    display.setCursor(90, 0);
    display.print("LED:ON");
  }
  else
  {
    display.setCursor(90, 0);
    display.print("LED:OFF");
  }

  display.setCursor(0, 12);
  if (showTemperature)
  {
    float t = dht.readTemperature();
    if (!isnan(t))
    {
      display.print("Nhiet do: ");
      display.print(t, 1);
      display.println(" C");
    }
  }
  else
  {
    display.print("Uptime: ");
    display.print(uptimeSeconds);
    display.println(" s");
  }

  int gas = random(320, 780); // hoặc đọc analogRead nếu có thật
  display.setCursor(0, 28);
  display.print("Khi gas: ");
  display.print(gas);
  display.println(" ppm");

  display.setCursor(0, 44);
  display.print("Do am: ");
  display.print(dht.readHumidity(), 1);
  display.println(" %");

  display.setCursor(0, 56);
  display.println("Team X - 2026");

  display.display();
  showTemperature = !showTemperature; // Luân phiên mỗi lần gọi
}

// ── Xử lý lệnh Telegram ────────────────────────
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized!", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    if (text == "/start")
    {
      String welcome = "Xin chào " + from_name + "!\n";
      welcome += "Nhóm IoT - Team X\n\n";
      welcome += "/on     - Bật LED\n";
      welcome += "/off    - Tắt LED\n";
      welcome += "/state  - Xem trạng thái LED\n";
      welcome += "/status - Xem nhiệt độ, độ ẩm, gas\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    else if (text == "/on")
    {
      ledState = true;
      updateLedState();
      bot.sendMessage(chat_id, "Đã bật LED 💡", "");
    }
    else if (text == "/off")
    {
      ledState = false;
      updateLedState();
      bot.sendMessage(chat_id, "Đã tắt LED", "");
    }
    else if (text == "/state" || text == "/get_state")
    {
      String st = ledState ? "LED đang BẬT" : "LED đang TẮT";
      bot.sendMessage(chat_id, st, "");
    }
    else if (text == "/status")
    {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      int g = random(320, 780); // hoặc analogRead
      String rep = "Trạng thái hiện tại:\n";
      rep += "Nhiệt độ: " + String(t, 1) + " °C\n";
      rep += "Độ ẩm: " + String(h, 1) + " %\n";
      rep += "Khí gas: " + String(g) + " ppm\n";
      rep += "LED: " + String(ledState ? "BẬT" : "TẮT");
      bot.sendMessage(chat_id, rep, "");
    }
  }
}

// ── SETUP ──────────────────────────────────────
void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  // pinMode(MQ2_PIN, INPUT);   // analog không cần pinMode

  digitalWrite(LED_PIN, LOW);

  dht.begin();

  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("OLED SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();

  // Kết nối WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.syncVirtual(V4);

  // Telegram client
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  bot.sendMessage(CHAT_ID, "Thiết bị IoT - Team X đã khởi động!");

  // Timer
  timer.setInterval(3000L, sendSensorData); // Đọc cảm biến + gửi thông báo
  timer.setInterval(4000L, updateOLED);     // Cập nhật OLED
}

// ── LOOP ───────────────────────────────────────
void loop()
{
  Blynk.run();
  timer.run();

  updateButton();

  // Telegram polling
  if (millis() - lastTimeBotRan > bot_delay)
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

// ── Đồng bộ từ Blynk app (Switch V4) ───────────
BLYNK_WRITE(V4)
{
  ledState = param.asInt();
  Serial.println(ledState ? "Blynk → LED ON" : "Blynk → LED OFF");
  updateLedState();

  // Thông báo Telegram khi điều khiển từ Blynk (tuỳ chọn)
  // bot.sendMessage(CHAT_ID, ledState ? "LED bật từ Blynk" : "LED tắt từ Blynk", "");
}