#include "motor.h"
#include "config.h"

StepperMotor::StepperMotor(int p1, int p2, int p3, int p4, int stepsPerRev) {
  pin[0]=p1; pin[1]=p2; pin[2]=p3; pin[3]=p4;
  stepsPerRevolution = stepsPerRev;
  stepDelay = 3;
  rotating = false;
  targetSteps = 0;
  currentStep = 0;
}

void StepperMotor::init() {
  for (int i=0; i<4; i++) pinMode(pin[i], OUTPUT);
  for (int i=0; i<4; i++) digitalWrite(pin[i], LOW);
}

void StepperMotor::setSpeed(int delayMs) {
  stepDelay = constrain(delayMs, 1, 20);
}

void StepperMotor::startRotate(int steps, bool clockwise) {
  if (steps <= 0 || rotating) return;
  targetSteps = steps;
  direction = clockwise;
  rotating = true;
  lastStepTime = millis();
}

bool StepperMotor::isBusy() {
  return rotating;
}

int StepperMotor::getRemainingSteps() {
  return targetSteps;
}

void StepperMotor::update() {
  if (!rotating) return;
  if (targetSteps <= 0) {
    rotating = false;
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
  int seqIndex;
  if (direction) {
    seqIndex = currentStep % 4;
  } else {
    seqIndex = (3 - (currentStep % 4));
  }
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