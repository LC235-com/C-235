#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class StepperMotor {
public:
  StepperMotor(int p1, int p2, int p3, int p4, int stepsPerRev);
  void init();
  void startRotate(int steps, bool clockwise); // 启动旋转（非阻塞）
  void update();  // 必须在 loop() 中频繁调用，驱动步进序列
  bool isBusy();  // 是否正在旋转
  void setSpeed(int delayMs); // 设置每步延时（速度）

private:
  int pin[4];
  int stepsPerRevolution;
  int stepDelay;          // 每步延时（ms）
  int targetSteps;        // 剩余要走的步数
  int currentStep;        // 当前步进序列索引 (0~3)
  bool rotating;
  unsigned long lastStepTime;
  bool direction;         // true=顺时针
  void stepOnce();
};

#endif