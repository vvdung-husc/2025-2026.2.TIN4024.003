#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Telegram
String BOT_TOKEN = "8736947766:AAECk-uGtVkp3PHAKdyyh9uNiis44k-6wTI";
String CHAT_ID  = "-5160714911";

WiFiClientSecure client;

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000);

// Update tracking
long lastUpdateId = 0;

// Alarm
int alarmHour = -1;
int alarmMinute = -1;
bool alarmTriggered = false;

// -------------------
// Telegram
// -------------------
void sendTelegram(String message) {
  client.setInsecure();
  message.replace(" ", "%20");

  String url = "/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + message;

  if (client.connect("api.telegram.org", 443)) {
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: api.telegram.org");
    client.println("Connection: close");
    client.println();
  }
}

// -------------------
// Telegram
// -------------------
void checkTelegram() {
  client.setInsecure();

  String url = "/bot" + BOT_TOKEN + "/getUpdates?offset=" + String(lastUpdateId + 1);

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("GetUpdates failed");
    return;
  }

  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Connection: close");
  client.println();

  String response = "";
  while (client.connected()) {
    response += client.readString();
  }

  Serial.println(response);

  // -------------------
  // /time
  // -------------------
  if (response.indexOf("/time") != -1) {
    Serial.println("Command /time detected");

    timeClient.update();

    int h = timeClient.getHours();
    int m = timeClient.getMinutes();
    int s = timeClient.getSeconds();

    String msg = "Time: ";
    msg += (h < 10 ? "0" : "") + String(h) + ":";
    msg += (m < 10 ? "0" : "") + String(m) + ":";
    msg += (s < 10 ? "0" : "") + String(s);

    sendTelegram(msg);
  }

  // -------------------
  // /setupnoti HH:MM
  // -------------------
  int setIndex = response.indexOf("/setupnoti");
  if (setIndex != -1) {
    Serial.println("Command /setupnoti detected");

    int spaceIndex = response.indexOf(" ", setIndex);
    int colonIndex = response.indexOf(":", setIndex);

    if (spaceIndex != -1 && colonIndex != -1) {

      String hourStr = response.substring(spaceIndex + 1, colonIndex);
      String minStr  = response.substring(colonIndex + 1, colonIndex + 3);

      alarmHour = hourStr.toInt();
      alarmMinute = minStr.toInt();
      alarmTriggered = false;

      Serial.println("Hour: " + String(alarmHour));
      Serial.println("Minute: " + String(alarmMinute));

      String confirm = "Da dat thong bao luc ";
      confirm += (alarmHour < 10 ? "0" : "") + String(alarmHour) + ":";
      confirm += (alarmMinute < 10 ? "0" : "") + String(alarmMinute);

      sendTelegram(confirm);
    } else {
      sendTelegram("Sai dinh dang! Dung: /setupnoti HH:MM");
    }
  }

  // update_id
  int idx = response.lastIndexOf("\"update_id\":");
  if (idx != -1) {
    String sub = response.substring(idx + 12);
    lastUpdateId = sub.toInt();
  }
}

// -------------------
// Setup
// -------------------
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  Wire.begin(21, 22);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  timeClient.begin();

  // Gửi hướng dẫn
  String guide =
    "HUONG DAN SU DUNG:%0A"
    "/time - Xem thoi gian hien tai%0A"
    "/setupnoti HH:MM - Dat thong bao";

  sendTelegram(guide);
}

// -------------------
// Loop
// -------------------
void loop() {
  timeClient.update();

  int h = timeClient.getHours();
  int m = timeClient.getMinutes();
  int s = timeClient.getSeconds();

  // OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("%02d:%02d:%02d", h, m, s);
  display.display();

  // Check Telegram
  checkTelegram();

  // -------------------
  // CHECK ALARM
  // -------------------
  if (alarmHour != -1 && alarmMinute != -1) {
    if (h == alarmHour && m == alarmMinute && !alarmTriggered) {

      String msg = "Bay gio la ";
      msg += (h < 10 ? "0" : "") + String(h) + ":";
      msg += (m < 10 ? "0" : "") + String(m);

      sendTelegram(msg);

      alarmTriggered = true;
    }
  }

  delay(3000);
}