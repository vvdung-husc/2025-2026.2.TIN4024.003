
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Initialize Telegram BOT
#define BOTtoken "8640473722:AAFLqYnLY9jyXyJ-dTKSc7XOYpp2PugRsqw"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-5254069055" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// PIR sensor
const int motionSensor = 27;

// LED
const int ledPin = 5;

bool motionDetected = false;

bool blinkMode = false;
bool ledState = false;

unsigned long previousMillis = 0;
const long blinkInterval = 200;

unsigned long lastTimeBotRan;
const int botRequestDelay = 1000;


// Format string
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);

  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);

  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);

  va_end(vaArgsCopy);

  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);

  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);

  String s = buff;

  free(buff);

  return String(s);
}


// PIR interrupt
void IRAM_ATTR detectsMovement() {
  motionDetected = true;
}


// Telegram command handler
void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.println(text);

    if (text == "/start") {

      String welcome = "Xin chào.\n";
      welcome += "Sử dụng các lệnh sau để điều khiển LED.\n\n";
      welcome += "/led_on : bật đèn nhấp nháy\n";
      welcome += "/led_off : tắt đèn\n";
      welcome += "/get_state : trạng thái đèn\n";

      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {

      blinkMode = true;
      ledState = true;

      bot.sendMessage(chat_id, "LED bắt đầu nhấp nháy", "");
    }

    if (text == "/led_off") {

      blinkMode = false;
      ledState = false;

      digitalWrite(ledPin, LOW);

      bot.sendMessage(chat_id, "LED đã tắt", "");
    }

    if (text == "/get_state") {

      if (blinkMode) {
        bot.sendMessage(chat_id, "LED đang nhấp nháy", "");
      } 
      else {
        bot.sendMessage(chat_id, "LED đang tắt", "");
      }
    }
  }
}


void setup() {

  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  Serial.print("Connecting Wifi: ");

  WiFi.begin(ssid, password);

  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi connected");

  bot.sendMessage(GROUP_ID, "Bot đã trực tuyến!", "");
}


void loop() {

  static uint count_ = 0;

  // PIR phát hiện chuyển động
  if(motionDetected){

    count_++;

    String msg = StringFormat("%u => Motion detected!", count_);

    bot.sendMessage(GROUP_ID, msg.c_str());

    Serial.println("Motion detected gửi Telegram");

    motionDetected = false;
  }


  // LED nhấp nháy
  if (blinkMode) {

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= blinkInterval) {

      previousMillis = currentMillis;

      ledState = !ledState;

      digitalWrite(ledPin, ledState);
    }
  }


  // đọc tin nhắn Telegram
  if (millis() - lastTimeBotRan > botRequestDelay) {

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}