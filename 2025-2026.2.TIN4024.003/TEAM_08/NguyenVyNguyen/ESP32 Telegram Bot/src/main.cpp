#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Thay đổi mạng WiFi của bạn
char ssid[] = "Wokwi-GUEST"; // Tên mạng WiFi
char password[] = "";        // Mật khẩu mạng WiFi

// Khai báo Bot Telegram
#define BOTtoken "8737511683:AAFyh-DV00cVj5geNgFAilpGCjzTl6yN1_8" // Thay thế thành Bot token của bạn

#define CHAT_ID "8642228083" // GROUP ID của bạn

// #define CHAT_ID2 "8590767456" // ID người dùng của bạn

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Kiểm tra có tin nhắn mới hay không liên tục sau mỗi 1 giây
int bot_delay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

// Thực thi lệnh khi có tin nhắn mới
void handleNewMessages(int numNewMessages)
{
  Serial.println("Handling New Message");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    // Chat ID của yêu cầu
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // In ra tin nhắn nhận được
    String user_text = bot.messages[i].text;
    Serial.println(user_text);

    String your_name = bot.messages[i].from_name;

    if (user_text == "/start")
    {
      String welcome = "Welcome, " + your_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "Send /led2_on to turn GPIO2 ON \n";
      welcome += "Send /led2_off to turn GPIO2 OFF \n";
      welcome += "Send /get_state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (user_text == "/led2_on")
    {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }

    if (user_text == "/led2_off")
    {
      bot.sendMessage(chat_id, "LED state is set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }

    if (user_text == "/get_state")
    {
      if (digitalRead(ledPin))
      {
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else
      {
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org"); // Nhận thời gian thực UTC qua NTP
  client.setTrustAnchors(&cert);    // Thêm chứng nhận root cho api.telegram.org
#endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  // Kết nối WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Thêm chứng nhận root cho api.telegram.org
#endif
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // In địa chỉ IP Local của ESP32
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "IoT Developer started up");
  // bot.sendMessage(CHAT_ID2, "IoT Developer started up123123");
}

void loop()
{
  if (millis() > lastTimeBotRan + bot_delay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("Got Response!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}