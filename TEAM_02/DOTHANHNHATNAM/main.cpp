#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= THÔNG SỐ MẠNG & BẢO MẬT =================
// 1. Cấu hình Wi-Fi (Dùng mạng ảo của Wokwi)
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

// 2. Cấu hình Telegram
#define BOT_TOKEN "8736049174:AAEW4WjaUGsvTebfmYHwOF6vovbZMf33x2Y"
#define CHAT_ID "5135985861"

// 3. Cấu hình HiveMQ Cloud
const char* mqtt_server = "5fff9cb1cb8f482d940e3acd7cf1485f.s1.eu.hivemq.cloud"; // VD: xxxxx.s1.eu.hivemq.cloud
const int mqtt_port = 8883; // Bắt buộc dùng 8883 cho HiveMQ Cloud
const char* mqtt_user = "nhatnam13112004"; // Username bạn đã tạo
const char* mqtt_pass = "Anhnam12122002@";

// Tạo 2 client bảo mật riêng biệt để không bị xung đột
WiFiClientSecure telegramClient;
UniversalTelegramBot bot(BOT_TOKEN, telegramClient);

WiFiClientSecure mqttClientSecure;
PubSubClient mqttClient(mqttClientSecure);

// ================= CẤU HÌNH PHẦN CỨNG =================
const int pHPin = 34;       
const int lightPin = 35;    
const int tempPin = 4;      

const int pumpLed = 25;     
const int lightLed = 26;    
const int phPumpLed = 27;   

const float TEMP_MAX = 28.0; 
const int LIGHT_MIN = 2000; 
const float PH_MIN = 5.5;
const float PH_MAX = 6.5;

OneWire oneWire(tempPin);
DallasTemperature sensors(&oneWire);

// Biến đếm thời gian (Hẹn giờ đa nhiệm)
unsigned long lastSensorRead = 0;
unsigned long lastTelegramSent = 0;
const long SENSOR_INTERVAL = 2000;   // 2 giây đọc cảm biến 1 lần
const long TELEGRAM_INTERVAL = 15000;// 15 giây gửi báo cáo Telegram 1 lần

// ================= HÀM KẾT NỐI MQTT =================
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Dang ket noi MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Thanh cong!");
    } else {
      Serial.print("Loi, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Thu lai sau 5 giay");
      delay(5000);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  sensors.begin();

  pinMode(pumpLed, OUTPUT);
  pinMode(lightLed, OUTPUT);
  pinMode(phPumpLed, OUTPUT);

  digitalWrite(pumpLed, LOW);
  digitalWrite(lightLed, LOW);
  digitalWrite(phPumpLed, LOW);

  // 1. Kết nối Wi-Fi
  Serial.print("Dang ket noi Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWi-Fi da ket noi!");

  // 2. Bỏ qua xác minh chứng chỉ SSL để tránh lỗi khi nạp lên ESP32
  telegramClient.setInsecure();
  mqttClientSecure.setInsecure();

  // 3. Cài đặt MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);

  // Gửi thông báo khởi động lên Telegram
  bot.sendMessage(CHAT_ID, "🚀 Hệ thống Vườn rau Thủy canh đã khởi động!", "");
}

// ================= VÒNG LẶP CHÍNH =================
void loop() {
  // Duy trì kết nối MQTT
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Đọc dữ liệu và xử lý định kỳ mỗi 2 giây
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = millis();

    // 1. Đọc dữ liệu
    sensors.requestTemperatures();
    float waterTemp = sensors.getTempCByIndex(0);
    int lightValue = analogRead(lightPin);
    int phRaw = analogRead(pHPin);
    float phValue = phRaw * (14.0 / 4095.0);

    Serial.printf("Nhiet do: %.2f *C | Anh sang: %d | pH: %.2f\n", waterTemp, lightValue, phValue);

    // 2. Logic điều khiển thiết bị
    if (waterTemp > TEMP_MAX) digitalWrite(pumpLed, HIGH); else digitalWrite(pumpLed, LOW);
    if (lightValue < LIGHT_MIN) digitalWrite(lightLed, HIGH); else digitalWrite(lightLed, LOW);
    if (phValue < PH_MIN || phValue > PH_MAX) digitalWrite(phPumpLed, HIGH); else digitalWrite(phPumpLed, LOW);

    // 3. Đẩy dữ liệu lên HiveMQ Cloud (Publish MQTT)
    mqttClient.publish("vuonrau/nhietdo", String(waterTemp).c_str());
    mqttClient.publish("vuonrau/anhsang", String(lightValue).c_str());
    mqttClient.publish("vuonrau/ph", String(phValue).c_str());
    
    // Gửi thêm trạng thái Relay lên MQTT để giám sát
    mqttClient.publish("vuonrau/relay/bomlammat", (waterTemp > TEMP_MAX) ? "ON" : "OFF");
    mqttClient.publish("vuonrau/relay/den", (lightValue < LIGHT_MIN) ? "ON" : "OFF");
    mqttClient.publish("vuonrau/relay/bomph", (phValue < PH_MIN || phValue > PH_MAX) ? "ON" : "OFF");
  }

  // Gửi báo cáo định kỳ lên Telegram mỗi 15 giây
  if (millis() - lastTelegramSent >= TELEGRAM_INTERVAL) {
    lastTelegramSent = millis();

    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);
    float currentPH = analogRead(pHPin) * (14.0 / 4095.0);
    
    String msg = "📊 **BÁO CÁO ĐỊNH KỲ:**\n";
    msg += "💧 Nhiệt độ: " + String(currentTemp, 1) + " °C\n";
    msg += "🧪 Độ pH: " + String(currentPH, 1) + "\n";
    
    // Thêm cảnh báo nếu thông số vượt ngưỡng
    if (currentTemp > TEMP_MAX) msg += "⚠️ Bơm làm mát đang BẬT\n";
    if (currentPH < PH_MIN || currentPH > PH_MAX) msg += "⚠️ Bơm bù pH đang BẬT\n";

    bot.sendMessage(CHAT_ID, msg, "Markdown");
    Serial.println("Đã gửi tin nhắn Telegram!");
  }
}