
#define BLYNK_TEMPLATE_ID "TMPL6C18sNN0e"
#define BLYNK_TEMPLATE_NAME "He thong dem nguoi"
#define BLYNK_AUTH_TOKEN "jDUhNaoMe1i0Rpl38Za9UQh3-x0s3e_D"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define IR1_PIN 14 // Nút xanh lá (Cảm biến ngoài)
#define IR2_PIN 27 // Nút màu xanh dương (Cảm biến trong)

int count = 0;     
int flag = 0;      

void setup() {
  Serial.begin(115200);
  pinMode(IR1_PIN, INPUT_PULLUP);
  pinMode(IR2_PIN, INPUT_PULLUP);
  
  Serial.println("Dang ket noi WiFi va Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "", "blynk.cloud", 80);
  Serial.println("He thong san sang!");
}

void loop() {
  Blynk.run(); 

  int state1 = digitalRead(IR1_PIN);
  int state2 = digitalRead(IR2_PIN);

  // --- ĐI VÀO ---
  if (flag == 0 && state1 == LOW && state2 == HIGH) {
    flag = 1; 
    Serial.println("Phat hien nguoi dang Vao...");
  }
  if (flag == 1 && state2 == LOW) {
    count++; 
    Serial.print("Mot nguoi da VAO. Tong so: "); Serial.println(count);
    Blynk.virtualWrite(V0, count); 
    flag = 0; 
    
    // Chống nhiễu
    while(digitalRead(IR1_PIN) == LOW || digitalRead(IR2_PIN) == LOW) {
      Blynk.run(); // Giữ kết nối
      delay(10);
    }
    Serial.println("San sang dem tiep.");
  }

  // --- ĐI RA ---
  if (flag == 0 && state2 == LOW && state1 == HIGH) {
    flag = 2; 
    Serial.println("Phat hien nguoi dang Ra...");
  }
  if (flag == 2 && state1 == LOW) {
    if (count > 0) count--; 
    Serial.print("Mot nguoi da RA. Tong so: "); Serial.println(count);
    Blynk.virtualWrite(V0, count); 
    flag = 0; 
    
    while(digitalRead(IR1_PIN) == LOW || digitalRead(IR2_PIN) == LOW) {
      Blynk.run();
      delay(10);
    }
    Serial.println("San sang dem tiep.");
  }
}