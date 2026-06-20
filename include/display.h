#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

extern Adafruit_SSD1306 display; // 全局对象，在main中定义

// 焦点枚举
enum Focus {
  FOCUS_HOUR,
  FOCUS_MIN,
  FOCUS_SEC,
  FOCUS_ACTION
};

// 操作按钮类型
enum ActionType {
  ACTION_START,
  ACTION_PAUSE,
  ACTION_RESET
};

// 状态
enum TimerState {
  STATE_IDLE,
  STATE_RUNNING,
  STATE_PAUSED,
  STATE_ALARM
};

void initDisplay();
void drawUI(int remainingSeconds, Focus focus, ActionType action, TimerState state);

#endif