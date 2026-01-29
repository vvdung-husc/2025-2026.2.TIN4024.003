#include <Arduino.h>

// ===== PIN (KHỚP DIAGRAM.JSON) =====
#define RED_PIN     25
#define YELLOW_PIN  33
#define GREEN_PIN   32

#define EXTRA_LED   21
#define BUTTON_PIN  23
#define SENSOR_PIN  34

#define CLK 15
#define DIO 2

// ===== TM1637 DRIVER TỐI GIẢN =====
const uint8_t segMap[] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66,
  0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

void tmStart() {
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);
}

void tmWriteByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    digitalWrite(DIO, b & 0x01);
    digitalWrite(CLK, HIGH);
    b >>= 1;
  }
  digitalWrite(CLK, LOW);
  pinMode(DIO, INPUT);
  digitalWrite(CLK, HIGH);
  pinMode(DIO, OUTPUT);
}

void tmCommand(uint8_t cmd) {
  tmStart();
  tmWriteByte(cmd);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);
}

void tmDisplay(uint16_t num) {
  tmCommand(0x40);
  tmStart();
  tmWriteByte(0xC0);
  tmWriteByte(segMap[num / 1000 % 10]);
  tmWriteByte(segMap[num / 100 % 10]);
  tmWriteByte(segMap[num / 10 % 10]);
  tmWriteByte(segMap[num % 10]);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);
  tmCommand(0x88 | 0x0f);
}

// ===== LOGIC =====
int redTime = 10;
int greenTime = 10;
int yellowTime = 3;
bool nightMode = false;

void setRed() {
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
}

void setGreen() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
}

void setYellow() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
}

void countdown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    tmDisplay(i);
    digitalWrite(EXTRA_LED, !digitalRead(EXTRA_LED));
    delay(1000);

    if (digitalRead(BUTTON_PIN) == LOW) {
      nightMode = !nightMode;
      delay(300);
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(EXTRA_LED, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT);

  tmCommand(0x8f); // brightness max
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);
  Serial.println(sensorValue);

  int base = map(sensorValue, 0, 4095, 5, 15);
  redTime = base;
  greenTime = base;
  yellowTime = 3;

  if (!nightMode) {
    setRed();    countdown(redTime);
    setGreen();  countdown(greenTime);
    setYellow(); countdown(yellowTime);
  } else {
    tmDisplay(0);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    digitalWrite(YELLOW_PIN, HIGH);
    delay(500);
    digitalWrite(YELLOW_PIN, LOW);
    delay(500);

    if (digitalRead(BUTTON_PIN) == LOW) {
      nightMode = false;
      delay(300);
    }
  }
}
