#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define BLYNK_TEMPLATE_ID "TMPL6TavIkQ3C"
#define BLYNK_TEMPLATE_NAME "BaiThiIOT"
#define BLYNK_AUTH_TOKEN "Wjm1uhHqKQoITiNUSC3sp8SNeY3REDrF"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Wokwi-GUEST";  
char pass[] = "";             

#define SENSOR_PIN 34 
#define PUMP_PIN 4    
#define BUZZER_PIN 5  

#define DRY_THRESHOLD 30    
#define WET_THRESHOLD 60    
#define DANGER_THRESHOLD 10 

LiquidCrystal_I2C lcd(0x27, 16, 2);

ulong currentMiliseconds = 0; 
bool pumpON = false;       
bool isNotified = false;   
int moisturePercent = 0;   

bool IsReady(ulong &ulTimer, uint32_t milisecond);
void readSensorAndLogic();
void updateLCD();
void handleBuzzer();
void syncBlynkData();

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Dang ket noi...");
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  lcd.clear();
}

void loop() {  
  Blynk.run();  
  currentMiliseconds = millis();

  readSensorAndLogic(); 
  handleBuzzer();       
  updateLCD();          
  syncBlynkData();      
}

bool IsReady(ulong &ulTimer, uint32_t milisecond) {
  if (currentMiliseconds - ulTimer < milisecond) return false;
  ulTimer = currentMiliseconds;
  return true;
}

void readSensorAndLogic() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 1000)) return; 

  int analogValue = analogRead(SENSOR_PIN);
  moisturePercent = map(analogValue, 0, 4095, 0, 100);

  if (moisturePercent < DRY_THRESHOLD) {
    if (!pumpON) {
      pumpON = true;
      digitalWrite(PUMP_PIN, HIGH);
      Blynk.virtualWrite(V1, 1);
    }
    if (!isNotified) {
      Blynk.logEvent("canh_bao_kho", "Canh bao: Dat dang rat kho!"); 
      isNotified = true; 
    }
  } 
  else if (moisturePercent > WET_THRESHOLD) {
    if (pumpON) {
      pumpON = false;
      digitalWrite(PUMP_PIN, LOW);
      Blynk.virtualWrite(V1, 0);
    }
    isNotified = false; 
  }
}

void handleBuzzer() {
  static ulong buzzerLastTime = 0;
  static bool buzzerState = LOW;
  if (moisturePercent <= DANGER_THRESHOLD) {
    if (IsReady(buzzerLastTime, 200)) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void updateLCD() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 500)) return;
  lcd.setCursor(0, 0);
  lcd.print("Do am: "); lcd.print(moisturePercent); lcd.print("%   ");
  lcd.setCursor(0, 1);
  if (moisturePercent <= DANGER_THRESHOLD) lcd.print("!! NGUY HIEM !! ");
  else if (pumpON) lcd.print("Trang thai: BOM ");
  else lcd.print("Trang thai: TOT ");
}

void syncBlynkData() {
  static ulong lastTime = 0;
  if (!IsReady(lastTime, 2000)) return;
  Blynk.virtualWrite(V0, moisturePercent); 
}

BLYNK_WRITE(V1) { 
  pumpON = param.asInt();
  digitalWrite(PUMP_PIN, pumpON ? HIGH : LOW);
}