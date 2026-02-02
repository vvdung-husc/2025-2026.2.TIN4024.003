/*
Thông tin nhóm 10:
1. Đinh Tuấn Anh.
2. Lê Trần Hải Đạt.
3. Phan Thanh Vũ.

*/
#include <Arduino.h>

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