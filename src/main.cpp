#include <Arduino.h>
#include "config.h"
#include "motor.h"
#include "display.h"
#include "buttons.h"
#include "buzzer.h"

// 全局对象
StepperMotor motor(MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4, STEP_PER_REV);

// 状态变量
TimerState state = STATE_IDLE;
Focus focus = FOCUS_MIN;      // 默认光标在分钟
ActionType action = ACTION_START;
int setSeconds = 600;          // 默认10分钟
int remainingSeconds = setSeconds;
bool timerRunning = false;
bool flipPending = false;      // 是否需要翻牌

// 时间更新
unsigned long lastSecondUpdate = 0;
unsigned long lastFlipTime = 0;

// 函数声明
void handleMove();
void handleInc();
void handleDec();
void handleOk();
void startTimer();
void pauseTimer();
void resetTimer();
void updateTimer();

void setup() {
  Serial.begin(115200);
  initDisplay();
  motor.init();
  motor.setSpeed(2);
  initButtons();
  initBuzzer();

  // 注册按钮回调
  setMoveCallback(handleMove);
  setIncCallback(handleInc);
  setDecCallback(handleDec);
  setOkCallback(handleOk);

  drawUI(remainingSeconds, focus, action, state);
}

void loop() {
  scanButtons();      // 处理按键事件
  motor.update();     // 驱动步进电机
  updateBuzzer();     // 更新闹铃

  // 倒计时更新
  if (timerRunning && state == STATE_RUNNING) {
    unsigned long now = millis();
    if (now - lastSecondUpdate >= 1000) {
      lastSecondUpdate = now;
      remainingSeconds--;
      if (remainingSeconds <= 0) {
        remainingSeconds = 0;
        timerRunning = false;
        state = STATE_ALARM;
        startAlarm();
        drawUI(remainingSeconds, focus, action, state);
        return;
      }
      // 检查是否到达整分钟（每分钟翻转一次）
      if (remainingSeconds % FLIP_INTERVAL == 0 && remainingSeconds > 0) {
        // 触发翻转（非阻塞）
        if (!motor.isBusy()) {
          motor.startRotate(STEP_PER_REV / 10, false); // 翻转一个叶片（1/10圈）
          // 因为你的翻牌器有10个叶片，每次翻1/10圈。如果想每次翻一片，可以每次转36度即2048/10=204.8步，取整205步。
          // 但你的电机步数可能不准，可以调整。这里取204步近似。
        }
      }
      drawUI(remainingSeconds, focus, action, state);
    }
  }
}

// ========== 按键回调 ==========
void handleMove() {
  if (state == STATE_ALARM) {
    resetTimer(); // 闹铃时按任意键停止闹铃并重置
    return;
  }
  // 移动焦点
  if (focus == FOCUS_SEC) focus = FOCUS_ACTION;
  else if (focus == FOCUS_ACTION) focus = FOCUS_HOUR;
  else focus = (Focus)(focus + 1);
  drawUI(remainingSeconds, focus, action, state);
}

void handleInc() {
  if (state == STATE_ALARM) { resetTimer(); return; }
  if (focus == FOCUS_ACTION) {
    // 切换操作按钮
    if (action == ACTION_RESET) action = ACTION_START;
    else action = (ActionType)(action + 1);
    drawUI(remainingSeconds, focus, action, state);
    return;
  }
  // 只有在IDLE状态才能改时间
  if (state != STATE_IDLE) return;
  int addSec = 0;
  if (focus == FOCUS_HOUR) addSec = 3600;
  else if (focus == FOCUS_MIN) addSec = 60;
  else if (focus == FOCUS_SEC) addSec = 1;
  if (addSec > 0) {
    setSeconds += addSec;
    if (setSeconds > MAX_SECONDS) setSeconds = MAX_SECONDS;
    remainingSeconds = setSeconds;
    drawUI(remainingSeconds, focus, action, state);
  }
}

void handleDec() {
  if (state == STATE_ALARM) { resetTimer(); return; }
  if (focus == FOCUS_ACTION) {
    if (action == ACTION_START) action = ACTION_RESET;
    else action = (ActionType)(action - 1);
    drawUI(remainingSeconds, focus, action, state);
    return;
  }
  if (state != STATE_IDLE) return;
  int decSec = 0;
  if (focus == FOCUS_HOUR) decSec = 3600;
  else if (focus == FOCUS_MIN) decSec = 60;
  else if (focus == FOCUS_SEC) decSec = 1;
  if (decSec > 0) {
    setSeconds -= decSec;
    if (setSeconds < 0) setSeconds = 0;
    remainingSeconds = setSeconds;
    drawUI(remainingSeconds, focus, action, state);
  }
}

void handleOk() {
  if (state == STATE_ALARM) {
    resetTimer();
    return;
  }
  if (focus == FOCUS_ACTION) {
    // 执行当前选中的操作
    if (action == ACTION_START) {
      if (state == STATE_IDLE || state == STATE_PAUSED) {
        startTimer();
      }
    } else if (action == ACTION_PAUSE) {
      if (state == STATE_RUNNING) {
        pauseTimer();
      }
    } else if (action == ACTION_RESET) {
      resetTimer();
    }
  } else {
    // 焦点在数字上，按确定自动跳到操作按钮（并选中START）
    focus = FOCUS_ACTION;
    action = ACTION_START;
    drawUI(remainingSeconds, focus, action, state);
  }
}

// ========== 定时器控制 ==========
void startTimer() {
  if (remainingSeconds <= 0) return;
  state = STATE_RUNNING;
  timerRunning = true;
  lastSecondUpdate = millis();
  // 如果当前剩余秒数是60的倍数，立即触发一次翻转（可选）
  if (remainingSeconds % FLIP_INTERVAL == 0 && remainingSeconds > 0) {
    if (!motor.isBusy()) {
      motor.startRotate(STEP_PER_REV / 10, false);
    }
  }
  drawUI(remainingSeconds, focus, action, state);
}

void pauseTimer() {
  state = STATE_PAUSED;
  timerRunning = false;
  drawUI(remainingSeconds, focus, action, state);
}

void resetTimer() {
  stopAlarm();
  timerRunning = false;
  state = STATE_IDLE;
  remainingSeconds = setSeconds;
  // 停止电机（如有转动）
  // 注意：motor没有stop接口，但我们可以忽略，因为当前转动会继续，但下次不会启动新的。
  drawUI(remainingSeconds, focus, action, state);
}