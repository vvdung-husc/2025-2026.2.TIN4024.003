#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPL61S_ZPvg1"
#define BLYNK_TEMPLATE_NAME "BaiTapIOT"
#define BLYNK_AUTH_TOKEN "_aA8UR_YVnUiyNlb3kN3F0GBuIL8HVnb"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>           


char ssid[] = "Wokwi-GUEST";  
char pass[] = "";             

#define PIN_LED_RED  21 
#define PIN_DHT      16 
#define DHT_TYPE     DHT22

DHT dht(PIN_DHT, DHT_TYPE); 
BlynkTimer timer;            

void connectWiFi() {
  Serial.println("Dang ket noi WiFi...");
  WiFi.begin(ssid, pass);
  

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8));
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi da ket noi!");
    Serial.print("Dia chi IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nLoi: Khong the ket noi WiFi!");
  }
}

void sendToBlynk() {

  unsigned long uptime = millis() / 1000;
  Serial.printf("Uptime: %lu seconds\n", uptime);
  Blynk.virtualWrite(V0, uptime);

 
  float nhietDo = dht.readTemperature();
  float doAm    = dht.readHumidity();

  if (isnan(nhietDo) || isnan(doAm)) {
    Serial.println("Loi doc cam bien DHT22!");
    return;
  }
  Serial.printf("Nhiet do: %.1f C - Do am: %.1f %%\n", nhietDo, doAm);
  Blynk.virtualWrite(V2, nhietDo); 
  Blynk.virtualWrite(V3, doAm);    
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);
  dht.begin();
  
 
  connectWiFi();
  
 
  IPAddress dns1(8, 8, 8, 8);      
  IPAddress dns2(8, 8, 4, 4);       
  WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns1, dns2);

 
  Serial.println("Dang ket noi Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  
  int attempts = 0;
  while (!Blynk.connected() && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (Blynk.connected()) {
    Serial.println("\nBlynk da ket noi!");
  } else {
    Serial.println("\nLoi: Khong the ket noi Blynk!");
  }

  timer.setInterval(5000L, sendToBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}


BLYNK_WRITE(V1) { 
 
  int value = param.asInt();  
  if (value == 1){
    printf("Blynk -> RED Light ON");
    digitalWrite(PIN_LED_RED, HIGH);
    
  }
  else {
    printf("Blynk -> RED Light OFF");
    digitalWrite(PIN_LED_RED, LOW);   
  }
}