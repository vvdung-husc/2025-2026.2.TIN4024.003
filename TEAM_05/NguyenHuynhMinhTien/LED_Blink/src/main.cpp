#include <Arduino.h>

// Define LED pin
const int ledPin = 17;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);  // Set LED pin as output
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin, HIGH);  // Turn LED on
  delay(1000);                 // Wait 1 second
  digitalWrite(ledPin, LOW);   // Turn LED off
  delay(1000);                 // Wait 1 second
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}