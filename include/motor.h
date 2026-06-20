#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class StepperMotor {
public:
  StepperMotor(int p1, int p2, int p3, int p4, int stepsPerRev);
  void init();
  void setSpeed(int delayMs);
  void startRotate(int steps, bool clockwise);
  bool isBusy();
  int getRemainingSteps();  // 获取剩余步数（用于转盘渐慢）
  void update();            // 必须在 loop 中频繁调用

private:
  int pin[4];
  int stepsPerRevolution;
  int stepDelay;
  int targetSteps;
  int currentStep;
  bool rotating;
  bool direction;
  unsigned long lastStepTime;
  void stepOnce();
};

#endif