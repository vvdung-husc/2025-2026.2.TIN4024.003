#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL6q7N9b6-s"
#define BLYNK_TEMPLATE_NAME "BlynkAPI"
#define BLYNK_AUTH_TOKEN "dmovmjdz2-ns9rizZ-HZM_axbAMWw3I1"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <TM1637Display.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// DHT22
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// TM1637
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// LED + Button
#define LED_PIN 2
#define BUTTON_PIN 4

// API KEY
#define OPENWEATHERMAP_KEY "9dc3891bec15c42c054d4cd0e6277621"

struct IP4_Info{
  String ip4;
  String latitude;
  String longitude;
};

IP4_Info ip4Info;

String urlWeather;

unsigned long currentMillis;

// timer
bool IsReady(unsigned long &timer, int ms){
  if(currentMillis - timer < ms) return false;
  timer = currentMillis;
  return true;
}

// phân tích GEO API
void parseGeoInfo(String payload){

  String values[7];
  int index=0;

  while(payload.length()>0 && index<7){

    int pos = payload.indexOf('|');

    if(pos==-1){
      values[index++] = payload;
      break;
    }

    values[index++] = payload.substring(0,pos);
    payload = payload.substring(pos+1);
  }

  ip4Info.ip4 = values[0];
  ip4Info.latitude = values[6];
  ip4Info.longitude = values[5];

  Serial.println("IP: "+ip4Info.ip4);
}

// lấy IP + location
void getAPI(){

  HTTPClient http;

  http.begin("http://ip4.iothings.vn/?geo=1");

  int httpCode = http.GET();

  Serial.print("GEO HTTP: ");
  Serial.println(httpCode);

  if(httpCode>0){

    String payload = http.getString();

    parseGeoInfo(payload);

    urlWeather =
    "https://api.openweathermap.org/data/2.5/weather?lat="+
    ip4Info.latitude+
    "&lon="+
    ip4Info.longitude+
    "&appid="+
    OPENWEATHERMAP_KEY+
    "&units=metric";

    Serial.println(urlWeather);
  }

  http.end();
}

// cập nhật nhiệt độ weather
void updateWeather(){

  static unsigned long lastTime=0;

  if(!IsReady(lastTime,10000)) return;

  if(urlWeather=="") return;

  HTTPClient http;

  http.begin(urlWeather);

  int httpCode = http.GET();

  Serial.print("Weather HTTP: ");
  Serial.println(httpCode);

  if(httpCode>0){

    String payload = http.getString();

    Serial.println(payload);

    DynamicJsonDocument doc(2048);

    DeserializationError error = deserializeJson(doc,payload);

    if(error){
      Serial.println("JSON error");
      return;
    }

    float temp = doc["main"]["temp"];

    Serial.print("Weather Temp: ");
    Serial.println(temp);

    Blynk.virtualWrite(V3,temp);
  }

  http.end();
}

// đọc DHT22
void readDHT(){

  static unsigned long lastTime=0;

  if(!IsReady(lastTime,2000)) return;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if(isnan(temp) || isnan(hum)){
    Serial.println("DHT Error");
    return;
  }

  Serial.print("DHT Temp: ");
  Serial.println(temp);

  Blynk.virtualWrite(V4,temp);
  Blynk.virtualWrite(V5,hum);

  display.showNumberDec((int)temp,true);
}

// uptime
void uptime(){

  static unsigned long lastTime=0;

  if(!IsReady(lastTime,1000)) return;

  Blynk.virtualWrite(V0,lastTime/1000);
}

// gửi IP và Map
void onceCalled(){

  static bool done=false;

  if(done) return;

  done=true;

  String link =
  "https://www.google.com/maps/place/"+
  ip4Info.latitude+
  ","+
  ip4Info.longitude;

  Blynk.virtualWrite(V1,ip4Info.ip4);
  Blynk.virtualWrite(V2,link);
}

// button LED
void buttonControl(){

  static int lastState=HIGH;

  int state=digitalRead(BUTTON_PIN);

  if(state==LOW && lastState==HIGH){

    digitalWrite(LED_PIN,!digitalRead(LED_PIN));
  }

  lastState=state;
}

void setup(){

  Serial.begin(115200);

  pinMode(LED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  dht.begin();

  display.setBrightness(7);

  WiFi.begin(WIFI_SSID,WIFI_PASSWORD,WIFI_CHANNEL);

  Serial.print("Connecting WiFi");

  while(WiFi.status()!=WL_CONNECTED){
    delay(200);
    Serial.print(".");
  }

  Serial.println(" Connected");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  getAPI();
}

void loop(){

  Blynk.run();

  currentMillis = millis();

  onceCalled();

  updateWeather();

  readDHT();

  uptime();

  buttonControl();
}