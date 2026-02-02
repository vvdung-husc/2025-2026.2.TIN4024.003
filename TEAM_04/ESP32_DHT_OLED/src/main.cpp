/*
THÔNG TIN NHÓM 4
1. Hồ Gia Long
2. Hồ Trọng Nghĩa
3. Phạm Văn Quân

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