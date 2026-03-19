#	/*
	THÔNG TIN NHÓM 05
	1. Nguyễn Huỳnh Minh Tiến 
	2. Lê Gia Huy
	3. Dương Hữu Tiến 
	4. Nguyễn Phú Tuấn
	5.
	*/

// Thêm thư viện chính
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "blynk_service.h"
#include "telegram_service.h"

#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>

// Telegram and EEPROM moved into telegram_service

#define LED_PIN LED_BUILTIN // GPIO2 (LED tích hợp trên NodeMCU)
#define DHTPIN 0                    // Chân DHT22
#define DHTTYPE DHT22               // Loại cảm biến DHT
#define MQ2_PIN A0                  // Chân analog cho MQ2 (khí gas)
#define OLED_SDA 4                  // Chân SDA cho OLED (D2 trên NodeMCU)
#define OLED_SCL 5                  // Chân SCL cho OLED (D1 trên NodeMCU)

// ====== CẤU HÌNH (đặt trong src/secrets.h, đã có mẫu) ======
#include "secrets.h"
// ====================================================

// Khởi tạo cảm biến và màn hình OLED
DHT dht(DHTPIN, DHTTYPE);
// Sử dụng U8g2 cho SH1106 (I2C)
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Biến lưu trạng thái và thời gian
float lastTemp = NAN, lastHum = NAN; // Lưu giá trị cảm biến trước đó
unsigned long lastSensorMillis = 0;   // thời điểm đọc cảm biến gần nhất
unsigned long startMillis;            // thời điểm khởi động để tính uptime

// Cấu hình khoảng thời gian và ngưỡng
const unsigned long SENSOR_INTERVAL = 2000;           // đọc DHT mỗi 2s
const unsigned long TELEGRAM_POLL_INTERVAL = 3000;    // kiểm tra Telegram mỗi 3s
const float TEMP_NOTIFY_DELTA = 0.5; // °C - ngưỡng thay đổi temp để thông báo
const float HUM_NOTIFY_DELTA = 1.0;  // %  - ngưỡng thay đổi độ ẩm để thông báo

// Blynk virtual pins
// V0: uptime (s)
// V1: LED switch (write from app)
// V2: temperature
// V3: humidity
// V4: gas
// V5: team info

// Blynk và Telegram timers/handlers nằm trong module tương ứng

// Hàm trợ giúp: định dạng thời gian hoạt động (uptime) thành HH:MM:SS
String formatUptime() {
	unsigned long s = (millis() - startMillis) / 1000;
	unsigned long h = s / 3600; s %= 3600;
	unsigned long m = s / 60; s %= 60;
	char buf[32];
	sprintf(buf, "%02lu:%02lu:%02lu", h, m, s);
	return String(buf);
}

// Cập nhật thông tin lên màn hình OLED
// Hiển thị: tên thiết bị, uptime, nhiệt độ, độ ẩm, khí gas, và thông tin team
void updateOLED(float t, float h, int gas) {
	oled.clearBuffer();
	oled.setFont(u8g2_font_unifont_t_vietnamese1);
	oled.drawUTF8(0, 12, "NodeMCU ESP8266");
	oled.setFont(u8g2_font_ncenB08_tr);
	char line[64];
	sprintf(line, "Uptime: %s", formatUptime().c_str());
	oled.drawStr(0, 26, line);
	sprintf(line, "Temp: %.1f C", t);
	oled.drawStr(0, 40, line);
	sprintf(line, "Hum:  %.1f %%", h);
	oled.drawStr(0, 52, line);
	// Hiển thị giá trị khí gas ở phía phải
	char gasLine[16];
	sprintf(gasLine, "%d", gas);
	oled.drawStr(90, 40, "Gas:");
	oled.drawStr(90, 52, gasLine);
	// Thông tin nhóm ở cuối màn hình
	oled.setFont(u8g2_font_unifont_t_vietnamese1);
	oled.drawUTF8(0, 64, "Team 05 - ESP8266 Blynk Telegram");
	oled.sendBuffer();
}

// Đọc giá trị MQ2; nếu không có sensor sẽ trả về số ngẫu nhiên để giả lập
int readMQ2OrRandom() {
	int val = analogRead(MQ2_PIN);
	if (val <= 0) {
		// Không có MQ2 -> trả về giá trị giả lập
		return (int)(random(200, 700));
	}
	return val;
}

// Gửi thông báo tới Telegram (gọi qua module)
void notifyTelegram(String text) {
	telegramSend(text);
}

// EEPROM handling moved to telegram_service

// Blynk send/update moved to blynk_service

// Telegram handling moved to telegram_service

// Hàm khởi tạo
void setup() {
	Serial.begin(115200);
	delay(100);
	pinMode(LED_PIN, OUTPUT);
	// Tắt LED ban đầu (đối với LED tích hợp thường là HIGH = tắt)
	digitalWrite(LED_PIN, HIGH);

	// Khởi tạo cảm biến và màn hình
	dht.begin();
	Wire.begin(OLED_SDA, OLED_SCL);
	oled.begin();
	oled.clearBuffer();

	// Kết nối WiFi
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	Serial.print("Connecting to WiFi");
	unsigned long tstart = millis();
	while (WiFi.status() != WL_CONNECTED && millis() - tstart < 20000) {
		Serial.print('.');
		delay(500);
	}
	Serial.println();
	if (WiFi.status() == WL_CONNECTED) {
		Serial.print("WiFi connected, IP: "); Serial.println(WiFi.localIP());
	} else {
		Serial.println("WiFi connect failed");
	}

	// Khởi tạo dịch vụ Blynk và Telegram
	blynkInit();
	telegramInit();

	// Thiết lập biến thời gian
	startMillis = millis();
	randomSeed(analogRead(A0));

	// Khi bot gửi lệnh điều khiển LED, telegram service sẽ gọi callback này
	telegramRegisterLedCallback([](bool on){ blynkSetLed(on); });

	// Hiển thị ban đầu
	updateOLED(0.0, 0.0, 0);
}

// Vòng lặp chính
void loop() {
	// Dịch vụ Blynk và Telegram xử lý nội bộ
	blynkTick();
	telegramTick();

	unsigned long now = millis();
	// Đọc cảm biến định kỳ
	if (now - lastSensorMillis >= SENSOR_INTERVAL) {
		lastSensorMillis = now;
		float hum = dht.readHumidity();
		float temp = dht.readTemperature();
		if (isnan(hum) || isnan(temp)) {
			Serial.println("DHT read failed");
		} else {
			Serial.printf("Temp: %.1f C  Hum: %.1f %%\n", temp, hum);
			int gas = readMQ2OrRandom();
			updateOLED(temp, hum, gas);

			// Gửi thông báo Telegram khi có thay đổi vượt ngưỡng
			bool notify = false;
			if (isnan(lastTemp) || fabs(temp - lastTemp) >= TEMP_NOTIFY_DELTA) notify = true;
			if (isnan(lastHum) || fabs(hum - lastHum) >= HUM_NOTIFY_DELTA) notify = true;
			if (notify) {
				char msg[128];
				sprintf(msg, "Sensor update:\nTemp: %.1f C\nHum: %.1f %%\nUptime: %s", temp, hum, formatUptime().c_str());
				notifyTelegram(String(msg));
			}
			lastTemp = temp; lastHum = hum;
			// Gửi dữ liệu lên Blynk
			blynkSendSensor(temp, hum, gas);
		}
	}
}
