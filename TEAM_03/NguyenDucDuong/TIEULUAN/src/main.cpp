
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <ESP_Mail_Client.h>


const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ===== Telegram =====
#define TELEGRAM_BOT_TOKEN "8699851099:AAHw1nZoS5lxVkmGiAhj_wPCZAUINcL7N_0"
#define TELEGRAM_CHAT_ID   "7004286968"
bool ENABLE_TELEGRAM = true;
bool SEND_TEST_TELEGRAM_ON_BOOT = true;


bool ENABLE_EMAIL = true; 
bool SEND_TEST_EMAIL_ON_BOOT = true;
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "ducduong10012004@gmail.com"
#define AUTHOR_PASSWORD "xxxxxxxxxxxxxxx"
#define RECIPIENT_EMAIL "ducviet28062015@gmail.com"


const int PIR_PIN = 13;
const int LED_PIN = 2; 

const unsigned long PIR_WARMUP_MS       = 5000;  
const unsigned long CONFIRM_MOTION_MS   = 500;   
const unsigned long ALERT_COOLDOWN_MS   = 30000; 
const unsigned long WIFI_RETRY_INTERVAL = 10000;


WiFiClientSecure secureClient;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, secureClient);

// Email objects
SMTPSession smtp;
Session_Config mailSession;

unsigned long bootTime = 0;
unsigned long motionHighStart = 0;
bool motionTimingStarted = false;
unsigned long lastAlertTime = 0;
unsigned long lastWifiRetry = 0;
int lastPirState = LOW;

bool isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void connectWiFi() {
  if (isWifiConnected()) return;

  Serial.printf("[WiFi] Connecting to %s ...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (!isWifiConnected() && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();

  if (isWifiConnected()) {
    Serial.print("[WiFi] Connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WiFi] Connection timeout.");
  }
}

void ensureWiFi() {
  if (!isWifiConnected() && millis() - lastWifiRetry >= WIFI_RETRY_INTERVAL) {
    lastWifiRetry = millis();
    connectWiFi();
  }
}

String getAlertMessage() {
  String msg = "🚨 CANH BAO XAM NHAP!\n";
  msg += "Phat hien chuyen dong bat thuong tu PIR.\n";
  msg += "Thoi gian uptime: ";
  msg += String(millis() / 1000);
  msg += " giay.";
  return msg;
}

bool sendTelegramAlert(const String& text) {
  if (!ENABLE_TELEGRAM) return false;
  if (!isWifiConnected()) return false;

  bool ok = bot.sendMessage(TELEGRAM_CHAT_ID, text, "");
  Serial.println(ok ? "[Telegram] Sent OK" : "[Telegram] Send failed");
  return ok;
}

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
}

bool sendEmailAlert(const String& text) {
  if (!ENABLE_EMAIL) return false;
  if (!isWifiConnected()) return false;

  static String appPassword;
  appPassword = AUTHOR_PASSWORD;
  appPassword.replace(" ", ""); 
  mailSession.server.host_name = SMTP_HOST;
  mailSession.server.port = SMTP_PORT;
  mailSession.login.email = AUTHOR_EMAIL;
  mailSession.login.password = appPassword.c_str();
  mailSession.login.user_domain = "";


  mailSession.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  mailSession.time.gmt_offset = 7;
  mailSession.time.day_light_offset = 0;

  SMTP_Message message;

  message.sender.name = "ESP32 Security";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "[ESP32 Alert] Phat hien chuyen dong";
  message.addRecipient("Owner", RECIPIENT_EMAIL);
  message.text.content = text.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  smtp.callback(smtpCallback);

  if (!smtp.connect(&mailSession)) {
    Serial.println("[Email] SMTP connect failed.");
    Serial.print("[Email] Reason: ");
    Serial.println(smtp.errorReason());
    return false;
  }

  bool ok = MailClient.sendMail(&smtp, &message);
  Serial.println(ok ? "[Email] Sent OK" : "[Email] Send failed");
  if (!ok) {
    Serial.print("[Email] Reason: ");
    Serial.println(smtp.errorReason());
  }
  smtp.closeSession();
  return ok;
}

void triggerAlert() {
  unsigned long now = millis();
  if (now - lastAlertTime < ALERT_COOLDOWN_MS) {
    Serial.println("[ALERT] In cooldown, skip.");
    return;
  }

  String msg = getAlertMessage();

  digitalWrite(LED_PIN, HIGH);
  delay(200);
  bool teleOk = sendTelegramAlert(msg);
  bool mailOk = sendEmailAlert(msg);
  digitalWrite(LED_PIN, LOW);

  if (teleOk || mailOk || (!ENABLE_TELEGRAM && !ENABLE_EMAIL)) {
    lastAlertTime = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("[BOOT] ESP32 Intrusion Alert booting...");
  Serial.printf("[CFG] WIFI_SSID = %s\n", WIFI_SSID);
  Serial.printf("[CFG] TELEGRAM  = %s\n", ENABLE_TELEGRAM ? "ON" : "OFF");
  Serial.printf("[CFG] EMAIL     = %s\n", ENABLE_EMAIL ? "ON" : "OFF");
  if (String(WIFI_SSID) == "YOUR_WIFI_SSID") {
    Serial.println("[WARN] Firmware dang dung SSID mau. Hay build lai dung project hien tai.");
  }

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  bootTime = millis();

  secureClient.setInsecure(); 

  connectWiFi();

  if (ENABLE_TELEGRAM && SEND_TEST_TELEGRAM_ON_BOOT && isWifiConnected()) {
    sendTelegramAlert("[BOOT] ESP32 da ket noi WiFi va san sang canh bao PIR.");
  }

  if (ENABLE_EMAIL && SEND_TEST_EMAIL_ON_BOOT && isWifiConnected()) {
    sendEmailAlert("[BOOT] Kết nối email thanh cong. ESP32 da san sang canh bao xam nhap.");
  }

  Serial.println("====================================");
  Serial.println("ESP32 Intrusion Alert System Started");
  Serial.println("PIR warm-up...");
  Serial.println("====================================");
}

void loop() {
  ensureWiFi();

  unsigned long now = millis();

  // Warm-up PIR
  if (now - bootTime < PIR_WARMUP_MS) {
    if ((now / 1000) % 5 == 0) {
      // log nhẹ theo nhịp, tránh spam console
    }
    delay(50);
    return;
  }

  int pirState = digitalRead(PIR_PIN);

  if (pirState != lastPirState) {
    Serial.printf("[PIR] State changed: %s\n", pirState == HIGH ? "HIGH" : "LOW");
    lastPirState = pirState;
  }

  if (pirState == HIGH) {
    if (!motionTimingStarted) {
      motionTimingStarted = true;
      motionHighStart = now;
    } else {
      if (now - motionHighStart >= CONFIRM_MOTION_MS) {
        Serial.println("[PIR] Motion confirmed.");
        triggerAlert();
        motionTimingStarted = false;
      }
    }
  } else {
    motionTimingStarted = false;
  }

  delay(50);
}