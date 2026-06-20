#include "motor.h"
#include "config.h"

StepperMotor::StepperMotor(int p1, int p2, int p3, int p4, int stepsPerRev) {
  pin[0]=p1; pin[1]=p2; pin[2]=p3; pin[3]=p4;
  stepsPerRevolution = stepsPerRev;
  stepDelay = 3;   // 默认3ms，可调
  rotating = false;
  targetSteps = 0;
  currentStep = 0;
}

void StepperMotor::init() {
  for (int i=0; i<4; i++) pinMode(pin[i], OUTPUT);
  // 初始全部置低
  for (int i=0; i<4; i++) digitalWrite(pin[i], LOW);
}

void StepperMotor::setSpeed(int delayMs) {
  stepDelay = constrain(delayMs, 1, 20);
}

void StepperMotor::startRotate(int steps, bool clockwise) {
  if (steps <= 0) return;
  targetSteps = steps;
  direction = clockwise;
  rotating = true;
  lastStepTime = millis();
  // 如果当前没有运行，立即执行第一步（可选）
}

bool StepperMotor::isBusy() {
  return rotating;
}

void StepperMotor::update() {
  if (!rotating) return;
  if (targetSteps <= 0) {
    rotating = false;
    // 所有线圈置低，省电
    for (int i=0; i<4; i++) digitalWrite(pin[i], LOW);
    return;
  }
  unsigned long now = millis();
  if (now - lastStepTime >= stepDelay) {
    lastStepTime = now;
    stepOnce();
    targetSteps--;
  }
}

void StepperMotor::stepOnce() {
  // 根据方向选择步进序列顺序
  int seqIndex;
  if (direction) {
    seqIndex = currentStep % 4;
  } else {
    seqIndex = (3 - (currentStep % 4));
  }
  // 四相八拍（半步）仅用4步？实际上你的motor.cpp只用了4种状态，那是全步模式（四相单四拍或双四拍），但你的代码里stepMotor只用了0-3，是单四拍？你用了LOW/HIGH组合，实际是两相通电（双四拍）。为了兼容你的代码，我们沿用你的步序表：
  // 你的stepMotor函数：case0: 0011, case1: 0110, case2: 1100, case3: 1001
  // 这就是双四拍（AB->BC->CD->DA），一圈需要4步？但28BYJ-48内部减速比，通常需要2048步才转一圈（半步模式）。你的stepsPerRevolution=2048，但只用了4步序列，那2048步相当于转了512个电周期？实际上你的代码中rotateClockwise是i%4循环，所以每个完整的电周期是4步，但2048步是512个周期。你的电机确实能转一圈（因为内部减速比）。所以我们就保持你的步序。
  // 我们直接使用你的case值。
  switch (seqIndex) {
    case 0: // 0011
      digitalWrite(pin[0], LOW); digitalWrite(pin[1], LOW);
      digitalWrite(pin[2], HIGH); digitalWrite(pin[3], HIGH);
      break;
    case 1: // 0110
      digitalWrite(pin[0], LOW); digitalWrite(pin[1], HIGH);
      digitalWrite(pin[2], HIGH); digitalWrite(pin[3], LOW);
      break;
    case 2: // 1100
      digitalWrite(pin[0], HIGH); digitalWrite(pin[1], HIGH);
      digitalWrite(pin[2], LOW); digitalWrite(pin[3], LOW);
      break;
    case 3: // 1001
      digitalWrite(pin[0], HIGH); digitalWrite(pin[1], LOW);
      digitalWrite(pin[2], LOW); digitalWrite(pin[3], HIGH);
      break;
  }
  currentStep++;
}