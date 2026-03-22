#define BLYNK_TEMPLATE_ID "TMPL6ZN2fkbAG"
#define BLYNK_TEMPLATE_NAME "ESP8266"
#define BLYNK_AUTH_TOKEN "STUdOfZNljLxElQSYwNJGr5efiSyXPj9"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- THÔNG TIN WIFI ---
char ssid[] = "Bao"; 
char pass[] = "12121212"; 

// --- THÔNG TIN TELEGRAM ---
#define BOTtoken "8590105919:AAHmtkplPldr-eTtt01UZk1W3b6ZjE5Wd9w"
#define CHAT_ID "-5118055885" 

// Cấu hình OLED (SCL -> D1, SDA -> D2)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Cấu hình DHT (D3)
#define DHTPIN 0 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

unsigned long lastTimeBotRan;
float temp, humid;
int gasValue;
float lastTempSent = 0;

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    if (text == "/led_on") {
      digitalWrite(LED_BUILTIN, LOW); 
      bot.sendMessage(CHAT_ID, "Đèn LED đã BẬT!", "");
    }
    else if (text == "/led_off") {
      digitalWrite(LED_BUILTIN, HIGH); 
      bot.sendMessage(CHAT_ID, "Đèn LED đã TẮT!", "");
    }
    else if (text == "/status") {
      String msg = "Nhiệt độ: " + String(temp) + " C\n";
      msg += "Độ ẩm: " + String(humid) + " %\n";
      msg += "Gas: " + String(gasValue) + " ppm";
      bot.sendMessage(CHAT_ID, msg, "");
    }
  }
}

BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  if (pinValue == 1) digitalWrite(LED_BUILTIN, LOW);
  else digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  Serial.begin(115200); 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED Fail");
  display.clearDisplay();
  display.setTextColor(WHITE);

  dht.begin();
  
  // ĐÃ SỬA: Sử dụng biến BLYNK_AUTH_TOKEN đã define ở đầu code
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  client.setInsecure(); 
}

void loop() {
  Blynk.run();
  
  temp = dht.readTemperature();
  humid = dht.readHumidity();
  gasValue = random(100, 500); 
  long uptime = millis() / 1000;

  Blynk.virtualWrite(V0, uptime);
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, humid);
  Blynk.virtualWrite(V4, gasValue);
  Blynk.virtualWrite(V5, "Team 9"); 

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("--- IoT TEAM 9 ---");
  display.printf("Temp: %.1f C\nHumid: %.1f %%\nGas: %d ppm\nUptime: %ld s", temp, humid, gasValue, uptime);
  display.display();

  if (millis() > lastTimeBotRan + 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    
    if (abs(temp - lastTempSent) > 1.0) {
       bot.sendMessage(CHAT_ID, "Cảnh báo: Nhiệt độ thay đổi thành " + String(temp) + " C", "");
       lastTempSent = temp;
    }
    lastTimeBotRan = millis();
  }
}