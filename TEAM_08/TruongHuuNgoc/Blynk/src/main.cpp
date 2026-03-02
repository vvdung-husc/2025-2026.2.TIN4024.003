#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPL6nmUCt10J"
#define BLYNK_TEMPLATE_NAME "Blynk and Wokwi"
#define BLYNK_AUTH_TOKEN "OOLJ5dTDApcDlg1gNwQ6yTxdFZTgW8nW"
// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}