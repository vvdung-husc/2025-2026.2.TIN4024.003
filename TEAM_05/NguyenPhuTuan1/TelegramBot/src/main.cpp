#include <Arduino.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";  // Add your WiFi password

// Initialize Telegram BOT
#define BOTtoken "8731978709:AAHc807C-Bn6ywAoyjpvq_bz2iHUOz3Mc9s"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-5158147486" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
const int ledPin = 23;       // LED Pin
bool motionDetected = false;
bool ledState = false;       // LED state

//Định dạng chuỗi %s,%d,...
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

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
}

// Handle Telegram messages
void handleNewMessages(int numNewMessages) {
  Serial.println("Handling new messages");
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    Serial.print("Received from chat_id: ");
    Serial.println(chat_id);
    Serial.print("Message: ");
    Serial.println(text);

    // Accept commands from any chat (not just GROUP_ID)
    // if(chat_id != String(GROUP_ID)){
    //   continue;
    // }

    if (text == "/on" || text == "/led_on") {
      digitalWrite(ledPin, HIGH);
      ledState = true;
      bot.sendMessage(chat_id, "LED bật ✅", "");
      Serial.println("LED ON");
    }
    else if (text == "/off" || text == "/led_off") {
      digitalWrite(ledPin, LOW);
      ledState = false;
      bot.sendMessage(chat_id, "LED tắt ❌", "");
      Serial.println("LED OFF");
    }
    else if (text == "/status" || text == "/get_state") {
      String status = ledState ? "LED đang BẬT ✅" : "LED đang TẮT ❌";
      bot.sendMessage(chat_id, status, "");
    }
    else if (text == "/start" || text == "/help") {
      String welcome = "Chào " + from_name + "!\n";
      welcome += "Sử dụng các lệnh sau:\n";
      welcome += "/on hoặc /led_on : Bật đèn\n";
      welcome += "/off hoặc /led_off : Tắt đèn\n";
      welcome += "/status hoặc /get_state : Kiểm tra trạng thái đèn\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else {
      // Echo unknown commands
      String response = "Không hiểu lệnh: " + text + "\nGửi /help để xem hướng dẫn";
      bot.sendMessage(chat_id, response, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Setup LED pin
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // LED off initially

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Optional - causes compile error
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Wait a bit before sending first message
  delay(2000);
  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}


void loop() {
  static uint count_ = 0;
  static unsigned long lastTime = 0;
  static int retryCount = 0;
  unsigned long now = millis();

  // Only process if WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected!");
    return;
  }

  // Check for new Telegram messages every 2 seconds with retry
  if (now - lastTime > 2000) {
    retryCount = 0;
    boolean success = false;
    
    while (retryCount < 3 && !success) {
      try {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        if (numNewMessages >= 0) {
          success = true;
          if (numNewMessages > 0) {
            Serial.print("Got ");
            Serial.print(numNewMessages);
            Serial.println(" messages");
            handleNewMessages(numNewMessages);
          }
        }
      } catch (...) {
        retryCount++;
        delay(500);
      }
    }
    
    if (!success && retryCount >= 3) {
      Serial.println("Failed to get updates after 3 retries");
    }
    
    lastTime = now;
  }

  if(motionDetected){
    ++count_;
    Serial.print(count_);Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!",count_);
    if(bot.sendMessage(GROUP_ID, msg.c_str())) {
      Serial.print(count_);Serial.println(". Sent successfully to Telegram: Motion Detected");
    } else {
      Serial.print(count_);Serial.println(". Failed to send to Telegram");
    }
    motionDetected = false;
  }
}