#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h> // Khai báo thư viện DHT

// ================= CẤU HÌNH PHẦN CỨNG =================
#define LDR_PIN 34   
#define LED_PIN 5    
#define DHTPIN 15     // Chân kết nối DHT22
#define DHTTYPE DHT22 // Loại cảm biến là DHT22

DHT dht(DHTPIN, DHTTYPE); // Khởi tạo đối tượng cảm biến DHT

// Cấu hình Wi-Fi & MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Các Topic MQTT 
const char* topic_pub_ldr  = "phanbahoa/sensor/ldr";
const char* topic_pub_temp = "phanbahoa/sensor/temperature"; // Topic gửi Nhiệt độ
const char* topic_pub_hum  = "phanbahoa/sensor/humidity";    // Topic gửi Độ ẩm
const char* topic_pub_led  = "phanbahoa/status/led";

const char* topic_sub_mode = "phanbahoa/control/mode"; 
const char* topic_sub_led  = "phanbahoa/control/led";  

WiFiClient espClient;
PubSubClient client(espClient);

bool isAutoMode = true; 
bool ledState = false;
unsigned long lastPublishTime = 0;

// ================= CÁC HÀM XỬ LÝ =================
void setup_wifi() {
  delay(10);
  Serial.print("\nĐang kết nối Wi-Fi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nĐã kết nối Wi-Fi!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  
  if (String(topic) == topic_sub_mode) {
    if (msg == "AUTO") { isAutoMode = true; Serial.println(">> Chế độ: TỰ ĐỘNG"); }
    else if (msg == "MANUAL") { isAutoMode = false; Serial.println(">> Chế độ: THỦ CÔNG"); }
  }

  if (String(topic) == topic_sub_led && !isAutoMode) {
    if (msg == "ON") { digitalWrite(LED_PIN, HIGH); ledState = true; Serial.println(">> Đèn: BẬT"); }
    else if (msg == "OFF") { digitalWrite(LED_PIN, LOW); ledState = false; Serial.println(">> Đèn: TẮT"); }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Kết nối MQTT...");
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" Thành công!");
      client.subscribe(topic_sub_mode);
      client.subscribe(topic_sub_led);
    } else {
      delay(5000);
    }
  }
}

// ================= SETUP & LOOP =================
void setup() {
  Serial.begin(115200);
  
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  dht.begin(); // Khởi động cảm biến DHT22

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  int ldrValue = analogRead(LDR_PIN);

  // LOGIC ĐÈN TỰ ĐỘNG
  if (isAutoMode) {
    if (ldrValue > 2000) { 
      if (!ledState) { digitalWrite(LED_PIN, HIGH); ledState = true; Serial.println(">> [AUTO] Trời tối -> BẬT đèn"); }
    } else {
      if (ledState) { digitalWrite(LED_PIN, LOW); ledState = false; Serial.println(">> [AUTO] Trời sáng -> TẮT đèn"); }
    }
  }

  // Gửi dữ liệu mỗi 2 giây
  unsigned long currentTime = millis();
  if (currentTime - lastPublishTime > 2000) {
    lastPublishTime = currentTime;

    // Đọc Nhiệt độ và Độ ẩm từ DHT22
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Kiểm tra xem cảm biến có đọc được dữ liệu không
    if (isnan(h) || isnan(t)) {
      Serial.println("Lỗi đọc dữ liệu từ DHT22!");
    } else {
      // Gửi dữ liệu MQTT
      client.publish(topic_pub_temp, String(t).c_str());
      client.publish(topic_pub_hum, String(h).c_str());
    }

    client.publish(topic_pub_ldr, String(ldrValue).c_str());
    client.publish(topic_pub_led, ledState ? "ON" : "OFF");
    
    // In ra Terminal để dễ theo dõi
    Serial.printf("Nhiệt độ: %.1f°C | Độ ẩm: %.1f%% | LDR: %d \n", t, h, ldrValue);
  }
}