#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

extern Adafruit_SSD1306 display;

enum AppMode {
  MODE_TIMER,
  MODE_DRUM,
  MODE_WHEEL
};

enum RowFocus {
  ROW_MODE,
  ROW_CONTENT,
  ROW_INSTRUMENT,
  ROW_ACTION
};

enum TimerState {
  STATE_IDLE,
  STATE_RUNNING,
  STATE_PAUSED,
  STATE_ALARM
};

// 全局变量声明（在 main.cpp 中定义）
extern int buzzerFreq;
extern int drumBPM;
extern bool inSettings;
extern int settingsFocus;

void initDisplay();
void drawUI(AppMode mode, RowFocus row, int contentFocus, 
            int remainingSeconds, TimerState timerState,
            bool drumPlaying, int drumCurrentPlayStep, bool drumPattern[DRUM_STEPS][DRUM_INSTRUMENTS],
            int drumEditInstrument, int wheelTarget, bool wheelSpinning);

#endif