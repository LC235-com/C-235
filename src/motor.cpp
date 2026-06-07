#include <Arduino.h>

// 定义连接到ULN2003的ESP32引脚
const int pin1 = 18;  // IN1接ESP32的GPIO18
const int pin2 = 19;  // IN2接ESP32的GPIO19
const int pin3 = 21;  // IN3接ESP32的GPIO21
const int pin4 = 22;  // IN4接ESP32的GPIO22

// 步进电机的步数
const int stepsPerRevolution = 2048;  // 28BYJ-48步进电机通常为2048步

// 旋转速度（延迟时间，单位：毫秒）
int rotationSpeed = 3;  // 设置全局变量旋转速度
int count=0;

// 初始化电机控制引脚
void setup() {
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  Serial.begin(115200);
  Serial.println("步进电机控制初始化完成");
}

// 步进电机控制函数
void stepMotor(int step) {
  switch (step) {
    case 0:  // 0011
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, LOW);
      digitalWrite(pin3, HIGH);
      digitalWrite(pin4, HIGH);
      break;
    case 1:  // 0110
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, HIGH);
      digitalWrite(pin3, HIGH);
      digitalWrite(pin4, LOW);
      break;
    case 2:  // 1100
      digitalWrite(pin1, HIGH);
      digitalWrite(pin2, HIGH);
      digitalWrite(pin3, LOW);
      digitalWrite(pin4, LOW);
      break;
    case 3:  // 1001
      digitalWrite(pin1, HIGH);
      digitalWrite(pin2, LOW);
      digitalWrite(pin3, LOW);
      digitalWrite(pin4, HIGH);
      break;
  }
}

// 顺时针旋转指定步数
void rotateClockwise(int steps) {
  for (int i = 0; i < steps; i++) {
    stepMotor(i % 4);
    delay(rotationSpeed);  // 使用全局变量控制旋转速度
  }
}

// 逆时针旋转指定步数
void rotateCounterClockwise(int steps) {
  for (int i = 0; i < steps; i++) {
    stepMotor((3 - (i % 4)));  // 反向控制顺序
    delay(rotationSpeed);  // 使用全局变量控制旋转速度
  }
}

// 旋转指定角度（正数表示顺时针，负数表示逆时针）
void rotateAngle(float degrees) {
  int steps = (int)(stepsPerRevolution * degrees / 360.0);
  if (steps > 0) {
    rotateClockwise(steps);
  } else if (steps < 0) {
    rotateCounterClockwise(-steps);
  }
}

// 主循环
void loop() {
  Serial.println("步进电机顺时针旋转一整圈...");
  rotateClockwise(stepsPerRevolution);
  delay(1000);

  Serial.println("步进电机逆时针旋转一整圈...");
  rotateCounterClockwise(stepsPerRevolution);
  delay(1000);

  Serial.println("步进电机旋转25度...");
  rotateAngle(25);
  delay(1000);
  count++;
  if(count == 3){
    delay(10000);
    count = 0;
  }
}
 
 