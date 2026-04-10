/*
 * ============================================================
 *  main.cpp
 *  Hệ thống đo nhịp tim và SpO2 qua ESP32
 *  Cảm biến: MAX30100 (mô phỏng vật lý bằng MAX30100_Sim.h)
 *  Nền tảng: Blynk IoT (blynk.cloud)
 *  Hiển thị: LCD 1602 I2C + 3 LED
 * ============================================================
 */

// ── [1] CẤU HÌNH BLYNK ───────────────────────────────────────
#define BLYNK_TEMPLATE_ID "TMPL6Xe3Yv623"
#define BLYNK_TEMPLATE_NAME "HeartRate Monitor"
#define BLYNK_AUTH_TOKEN "FK7Bx-LQDehEia6eRRMYP_x11vT505YG"// <-- đổi thành của bạn

// ── [2] THƯ VIỆN ─────────────────────────────────────────────
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HeartRate_ESP32\MAX30100_Sim.h"      // ← thư viện mô phỏng của chúng ta

// ── [3] THÔNG TIN WIFI ───────────────────────────────────────
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// ── [4] CHÂN PHẦN CỨNG ───────────────────────────────────────
#define PIN_LED_HEART   2
#define PIN_LED_STATUS  4
#define PIN_LED_WARN    5
#define I2C_SDA        21
#define I2C_SCL        22

// ── [5] ĐỐI TƯỢNG ────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
MAX30100Sim pox;                  // cảm biến mô phỏng
BlynkTimer  timer;

// ── [6] BIẾN TOÀN CỤC ────────────────────────────────────────
float bpm  = 0;
float spo2 = 0;

unsigned long lastLCDUpdate = 0;
unsigned long lastBlink     = 0;
bool ledHeartState          = false;

// ── [7] PHÂN LOẠI TRẠNG THÁI ─────────────────────────────────
String getStatus(float heartRate, float spO2) {
    if (!pox.isReady())     return "Warming up...";
    if (spO2   < 95.0f)     return "Thiếu SPO2!";
    if (heartRate < 60.0f)  return "Tim đập quá chậm!";
    if (heartRate > 100.0f) return "Tim đập nhanh!";
    return "Normal";
}

bool isWarning(float heartRate, float spO2) {
    return pox.isReady() && (heartRate < 60 || heartRate > 100 || spO2 < 95.0f);
}

// ── [8] GỬI DỮ LIỆU LÊN BLYNK ───────────────────────────────
void sendToBlynk() {
    if (!Blynk.connected() || !pox.isReady()) return;

    String status = getStatus(bpm, spo2);
    unsigned long sec = millis() / 1000;
    String ts = String(sec / 3600) + "h " +
                String((sec % 3600) / 60) + "m " +
                String(sec % 60) + "s";

    Blynk.virtualWrite(V0, (int)bpm);
    Blynk.virtualWrite(V1, spo2);
    Blynk.virtualWrite(V2, status);
    Blynk.virtualWrite(V3, ts);

    Serial.printf("[Blynk] BPM=%.1f  SpO2=%.1f%%  Status=%s\n",
                  bpm, spo2, status.c_str());
}

// ── [9] CẬP NHẬT LCD ─────────────────────────────────────────
void updateLCD() {
    lcd.clear();

    if (!pox.isReady()) {
        // Hiển thị thanh tiến trình warm-up
        int progress = (int)(pox.getWarmupProgress() * 16);
        lcd.setCursor(0, 0);
        lcd.print("Warming up...   ");
        lcd.setCursor(0, 1);
        for (int i = 0; i < 16; i++) {
            lcd.print(i < progress ? (char)255 : ' ');
        }
        return;
    }

    lcd.setCursor(0, 0);
    lcd.print("BPM:");
    if (bpm > 0) {
        lcd.print((int)bpm);
    } else {
        lcd.print("---");
    }

    if (isWarning(bpm, spo2)) {
        lcd.setCursor(8, 0); 
        lcd.print("[C.BAO]"); // In chữ Cảnh Báo lên góc phải màn hình
    }

    lcd.setCursor(0, 1);
    lcd.print("SpO2:");
    if (spo2 > 0) {
        lcd.print(spo2, 1);
        lcd.print("%");
    } else {
        lcd.print("--.-% ");
    }
}

// ── [10] SETUP ────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== He thong do nhip tim ESP32 ===");

    pinMode(PIN_LED_HEART,  OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_LED_WARN,   OUTPUT);

    // Khởi động LCD
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Ket noi WiFi...");

    // Khởi động cảm biến mô phỏng
    pox.begin();

    // Kết nối Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);

    bool ok = Blynk.connected();
    digitalWrite(PIN_LED_STATUS, ok ? HIGH : LOW);
    lcd.clear();
    lcd.print(ok ? "Blynk: OK" : "Blynk: FAIL");
    delay(1000);

    // Gửi Blynk mỗi 5 giây
    timer.setInterval(5000L, sendToBlynk);

    Serial.println("[System] San sang!");
}

// ── [11] LOOP ─────────────────────────────────────────────────
void loop() {
    Blynk.run();
    timer.run();

    // Cập nhật cảm biến – gọi liên tục, bên trong tự throttle 10ms
    pox.update();

    unsigned long now = millis();

    // Lấy giá trị từ cảm biến
    bpm  = pox.getHeartRate();
    spo2 = pox.getSpO2();

    // Cập nhật LCD mỗi 500ms
    if (now - lastLCDUpdate >= 500) {
        lastLCDUpdate = now;
        updateLCD();

        // Cập nhật LED cảnh báo
        digitalWrite(PIN_LED_WARN, isWarning(bpm, spo2) ? HIGH : LOW);
        Serial.printf("[Debug LED Vàng] Trang thai canh bao: %s\n", isWarning(bpm, spo2) ? "ON (D5 xuat HIGH)" : "OFF (D5 xuat LOW)");

        
        // Cập nhật LED kết nối
        digitalWrite(PIN_LED_STATUS, Blynk.connected() ? HIGH : LOW);

        // Debug tín hiệu thô
        if (pox.isReady()) {
            Serial.printf("[Sensor] BPM=%.1f  SpO2=%.1f%%  IR=%.0f  RED=%.0f  Status=%s\n",
                          bpm, spo2,
                          pox.getRawIR(), pox.getRawRed(),
                          getStatus(bpm, spo2).c_str());
        } else {
            Serial.printf("[Warmup] %.0f%%\n", pox.getWarmupProgress() * 100);
        }
    }

    // LED tim nhấp nháy theo BPM
    if (pox.isReady() && bpm > 0) {
        unsigned long blinkInterval = (unsigned long)(60000.0f / bpm / 2.0f);
        if (now - lastBlink >= blinkInterval) {
            lastBlink      = now;
            ledHeartState  = !ledHeartState;
            digitalWrite(PIN_LED_HEART, ledHeartState);
        }
    }
}

// ── [12] BLYNK CALLBACKS ─────────────────────────────────────
BLYNK_WRITE(V5) {
    if (param.asInt() == 1) {
        Serial.println("[Blynk] Reset!");
        lcd.clear();
        lcd.print("Resetting...");
        delay(500);
    }
}

BLYNK_CONNECTED() {
    Serial.println("[Blynk] Ket noi thanh cong!");
    digitalWrite(PIN_LED_STATUS, HIGH);
    Blynk.syncVirtual(V5);
}

BLYNK_DISCONNECTED() {
    Serial.println("[Blynk] Mat ket noi!");
    digitalWrite(PIN_LED_STATUS, LOW);
}
