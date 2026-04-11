#define BLYNK_TEMPLATE_ID "TMPL6HIROwqQ-"
#define BLYNK_TEMPLATE_NAME "BLYNKDHT"
#define BLYNK_AUTH_TOKEN "FtIVb7rO_HVbNVAEQBkyzRfivFTOPi70"
#define API "f09f4d820be844c8d24ddbe5a608fe95" 

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = ""; 

BlynkTimer timer; 

String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void updateNetworkData() {
  WiFiClient client;
  if (client.connect("103.179.173.252", 80)) {
    client.print("GET /?geo=1 HTTP/1.1\r\nHost: ip4.iothings.vn\r\nConnection: close\r\n\r\n");
    delay(500);
    String payload = "";
    while (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf('|') != -1) payload = line;
    }
    client.stop();

    if (payload != "") {
      String myIP = splitString(payload, '|', 0); 
      String lon  = splitString(payload, '|', 5); 
      String lat  = splitString(payload, '|', 6); 
      
      Blynk.virtualWrite(V2, myIP);
      Blynk.virtualWrite(V3, "https://www.google.com/maps/place/" + lat + "," + lon);
      
      // Lấy thời tiết (Rút gọn cho nhẹ)
      if (client.connect("15.235.227.40", 80)) {
        client.print(String("GET /data/2.5/weather?lat=") + lat + "&lon=" + lon + "&appid=" + API + "&units=metric HTTP/1.1\r\nHost: api.openweathermap.org\r\nConnection: close\r\n\r\n");
        delay(500);
        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line.startsWith("{")) {
            JsonDocument doc;
            deserializeJson(doc, line);
            Blynk.virtualWrite(V4, (float)doc["main"]["temp"]);
          }
        }
        client.stop();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }


  IPAddress blynkIP(128, 199, 144, 129);
  Blynk.config(BLYNK_AUTH_TOKEN, blynkIP, 80);
  Blynk.connect();

  updateNetworkData();
  timer.setInterval(1000L, [](){ Blynk.virtualWrite(V1, millis()/1000); });
  timer.setInterval(30000L, updateNetworkData); 
}

void loop() {
  Blynk.run();
  timer.run();
}