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
const char* password = "";

// Initialize Telegram BOT
#define BOTtoken "8519452760:AAHIU0zWrrUzzu7EomeEZlmYYVaLAsJ8It8"  // your Bot Token (Get from Botfather)

// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-8696067864" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
bool motionDetected = false;

// Telegram polling and LED control
int bot_delay = 1000; // ms between polling
unsigned long lastTimeBotRan = 0;

const int ledPin = 23; // Wokwi LED connected to GPIO23
bool ledState = LOW;

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

// Handle incoming Telegram messages (commands)
void handleNewMessages(int numNewMessages){
  for(int i=0; i<numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);
    String user_text = bot.messages[i].text;

    // Only accept commands from the configured GROUP_ID
    if(chat_id != String(GROUP_ID)){
      // ignore messages from other chats
      continue;
    }

    if(user_text == "/led_on"){
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED turned ON", "");
      Serial.println("Action: LED turned ON (from /led_on)");
    }
    else if(user_text == "/led_off"){
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      bot.sendMessage(chat_id, "LED turned OFF", "");
      Serial.println("Action: LED turned OFF (from /led_off)");
    }
    else if(user_text == "/get_state"){
      if(digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
        Serial.println("Action: Reported LED is ON (from /get_state)");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
        Serial.println("Action: Reported LED is OFF (from /get_state)");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // LED output init
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  
  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}


void loop() {
  static uint count_ = 0;

  // Poll Telegram for commands periodically
  if (millis() > lastTimeBotRan + bot_delay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  if(motionDetected){
    ++count_;
    Serial.print(count_);Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!",count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_);Serial.println(". Sent successfully to Telegram: Motion Detected");
    motionDetected = false;
  }
}

