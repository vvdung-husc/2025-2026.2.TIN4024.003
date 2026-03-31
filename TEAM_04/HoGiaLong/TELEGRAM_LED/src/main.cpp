    #include <Arduino.h>
    #include <WiFi.h>
    #include <WiFiClientSecure.h>
    #include <UniversalTelegramBot.h>
    #include <ArduinoJson.h>

    // Thay thế bằng thông tin WiFi của bạn (nếu chạy thật) hoặc để nguyên nếu chạy trên Wokwi
    const char* ssid = "Wokwi-GUEST";
    const char* password = "";

    // Điền Token và Group ID
    #define BOTtoken ""
    #define GROUP_ID ""

    WiFiClientSecure client;
    UniversalTelegramBot bot(BOTtoken, client);

    // Định nghĩa các chân GPIO
    const int motionSensor = 27; // Cảm biến PIR
    const int ledPin = 23;       // Đèn LED

    bool motionDetected = false;
    bool ledState = false;       // Biến lưu trạng thái hiện tại của LED

    // Thời gian trễ giữa các lần kiểm tra tin nhắn mới
    int botRequestDelay = 1000; 
    unsigned long lastTimeBotRan;

    // Hàm ngắt khi phát hiện chuyển động
    void IRAM_ATTR detectsMovement() {
      motionDetected = true;
    }

    // Hàm xử lý khi có tin nhắn mới gửi đến Bot
    void handleNewMessages(int numNewMessages) {
      Serial.print("Nhận được tin nhắn mới: ");
      Serial.println(numNewMessages);

      for (int i = 0; i < numNewMessages; i++) {
        // Lấy ID của người/nhóm chat và nội dung tin nhắn
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        String from_name = bot.messages[i].from_name;

        // SỬA LỖI LỆNH: Dùng startsWith() để bắt được cả lệnh thường lẫn lệnh có kèm @bot_name
        if (text.startsWith("/start")) {
          String welcome = "Xin chào, " + from_name + ".\n";
          welcome += "Sử dụng các lệnh sau để điều khiển đèn LED.\n\n";
          welcome += "Gửi /led_on để bật sáng đèn\n";
          welcome += "Gửi /led_off để tắt đèn\n";
          welcome += "Gửi /get_state để yêu cầu trạng thái đèn hiện tại\n";
          bot.sendMessage(chat_id, welcome, "");
        }
        else if (text.startsWith("/led_on")) {
          digitalWrite(ledPin, HIGH); // Bật LED thực tế
          ledState = true;            // Cập nhật biến trạng thái
          bot.sendMessage(chat_id, "LED bật sáng", "");
        }
        else if (text.startsWith("/led_off")) {
          digitalWrite(ledPin, LOW);  // Tắt LED thực tế
          ledState = false;
          bot.sendMessage(chat_id, "LED đã tắt", ""); 
        }
        else if (text.startsWith("/get_state")) {
          if (ledState) {
            bot.sendMessage(chat_id, "LED is ON", "");
          } else {
            bot.sendMessage(chat_id, "LED is OFF", "");
          }
        }
      }
    }

    void setup() {
      Serial.begin(115200);

      // Cấu hình chân cảm biến và ngắt
      pinMode(motionSensor, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

      // Cấu hình chân LED
      pinMode(ledPin, OUTPUT);
      digitalWrite(ledPin, LOW); // Đảm bảo LED tắt khi mới khởi động

      // Kết nối WiFi
      Serial.print("Connecting Wifi: ");
      Serial.println(ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      
      // SỬA LỖI SPAM VÀ LỖI SSL: Dùng setInsecure() thay vì setCACert()
      client.setInsecure(); 
      
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
      }
      Serial.println("\nWiFi connected");
      
      // Gửi tin nhắn thông báo bot đã sẵn sàng
      bot.sendMessage(GROUP_ID, "IoT Developer started up. Gửi /start để xem các lệnh.");
    }

    void loop() {
      // 1. Xử lý logic cảnh báo chuyển động 
      if (motionDetected) {
        Serial.println("MOTION DETECTED => Sending to Telegram");    
        bot.sendMessage(GROUP_ID, "Phát hiện có chuyển động!");
        motionDetected = false;
      }

      // 2. Xử lý logic đọc lệnh điều khiển 
      if (millis() - lastTimeBotRan > botRequestDelay) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        
        while (numNewMessages) {
          Serial.println("Có lệnh điều khiển tới!");
          handleNewMessages(numNewMessages); 
          numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis(); 
      }
    }