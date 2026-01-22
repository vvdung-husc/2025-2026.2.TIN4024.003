#include <Arduino.h>

#define green_led  23
#define red_led    19 
#define yellow_led 18

void setup() {
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
}

void loop() {

  digitalWrite(red_led, LOW);   
  digitalWrite(yellow_led, LOW); 
  digitalWrite(green_led, HIGH); 
  delay(3000); 

  digitalWrite(green_led, LOW);
  digitalWrite(yellow_led, HIGH); 
  delay(1000); 

  digitalWrite(yellow_led, LOW); 
  digitalWrite(red_led, HIGH);   
  delay(3000); 
}