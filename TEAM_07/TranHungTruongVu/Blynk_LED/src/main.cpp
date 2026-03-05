#define BLYNK_TEMPLATE_ID "TMPL6c3MCY1bZ"
#define BLYNK_TEMPLATE_NAME "Blynk"
#define BLYNK_AUTH_TOKEN "TT-QgMv6URnnQaZocp7A-iLqWjKwivop"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <TM1637Display.h>

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

#define LED_PIN     21
#define BUTTON_PIN  23
#define CLK         18
#define DIO         19

TM1637Display display(CLK, DIO);
BlynkTimer timer;

bool ledState = false;
unsigned long startTime = 0;
unsigned long totalTime = 0;   // Tổng thời gian đã chạy
bool lastButtonState = HIGH;

// ================= BLYNK SWITCH =================
BLYNK_WRITE(V0)
{
  ledState = param.asInt();
  digitalWrite(LED_PIN, ledState);

  if (ledState) {
    startTime = millis();
  } else {
    totalTime += (millis() - startTime) / 1000;
  }
}

// ================= UPDATE TIMER =================
void updateTimer()
{
  unsigned long currentTime = totalTime;

  if (ledState) {
    currentTime += (millis() - startTime) / 1000;
  }

  display.showNumberDec(currentTime);
  Blynk.virtualWrite(V1, currentTime);
}

// ================= BUTTON =================
void checkButton()
{
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    Blynk.virtualWrite(V0, ledState);

    if (ledState) {
      startTime = millis();
    } else {
      totalTime += (millis() - startTime) / 1000;
    }

    delay(200);
  }

  lastButtonState = currentButtonState;
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  display.setBrightness(7);
  display.showNumberDec(0);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, updateTimer);
  timer.setInterval(50L, checkButton);
}

// ================= LOOP =================
void loop()
{
  Blynk.run();
  timer.run();
}