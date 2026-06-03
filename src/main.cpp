#include <Arduino.h>

int myFunction(int, int);
int i = 0;

void setup() {
  int result = myFunction(0, 6);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
}

void loop() {
  switch(i){
    case 0: digitalWrite(2, HIGH);
      delay(1000);
      digitalWrite(2, LOW);
      break;
    case 1: digitalWrite(4, HIGH); 
      delay(1000);
      digitalWrite(4, LOW);
      break;
    case 2: digitalWrite(5, HIGH);
      delay(1000);
      digitalWrite(5, LOW);
      break;
    case 3: digitalWrite(18, HIGH);
      delay(1000);
      digitalWrite(18, LOW);
      break;
    case 4: digitalWrite(19, HIGH);
      delay(1000);
      digitalWrite(19, LOW);
      break;
    case 5: digitalWrite(21, HIGH);
      delay(1000);
      digitalWrite(21, LOW);
      break;
  }
  delay(1000);
  if(i == 5){
    i = 0;
  } else {
    i++;
  }
}
int myFunction(int x, int y) {
  return x + y;
}
