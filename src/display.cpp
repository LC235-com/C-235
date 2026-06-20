#include "display.h"
#include "config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 外部变量定义（引用 main.cpp 中的全局变量）
extern int buzzerFreq;
extern int drumBPM;
extern bool inSettings;
extern int settingsFocus;

void initDisplay() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed!");
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.display();
}

static void drawNumberWithFocus(int x, int y, int num, bool hasFocus) {
  char buf[3];
  sprintf(buf, "%02d", num);
  if (hasFocus) {
    display.fillRect(x-2, y-2, 24, 18, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(x, y);
    display.print(buf);
    display.setTextColor(SSD1306_WHITE);
  } else {
    display.setCursor(x, y);
    display.print(buf);
  }
}

static void drawModeRow(AppMode mode, RowFocus row) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  if (row == ROW_MODE) {
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  }
  const char* modeNames[] = {"Timer", "Drum", "Wheel"};
  display.print("Mode: ");
  display.print(modeNames[(int)mode]);
  display.setTextColor(SSD1306_WHITE);
}

static void drawActionRow(RowFocus row, AppMode mode, TimerState timerState, bool drumPlaying) {
  display.setCursor(0, 54);
  display.setTextSize(1);
  if (row == ROW_ACTION) {
    display.fillRect(0, 52, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  }
  const char* actionText = "";
  if (mode == MODE_TIMER) {
    if (timerState == STATE_RUNNING) actionText = "Pause";
    else if (timerState == STATE_PAUSED || timerState == STATE_IDLE) actionText = "Start";
    else if (timerState == STATE_ALARM) actionText = "Reset";
  } else if (mode == MODE_DRUM) {
    actionText = drumPlaying ? "Stop" : "Play";
  } else if (mode == MODE_WHEEL) {
    actionText = "Spin";
  }
  display.print("Action: ");
  display.print(actionText);
  display.setTextColor(SSD1306_WHITE);
}

void drawUI(AppMode mode, RowFocus row, int contentFocus, 
            int remainingSeconds, TimerState timerState,
            bool drumPlaying, int drumCurrentPlayStep, bool drumPattern[DRUM_STEPS][DRUM_INSTRUMENTS],
            int drumEditInstrument, int wheelTarget, bool wheelSpinning) {
  display.clearDisplay();

  // ---- 设置页面绘制 ----
  if (inSettings) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Settings");
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print("Freq: ");
    display.print(buzzerFreq);
    display.println(" Hz");
    display.print("BPM:  ");
    display.print(drumBPM);

    // 高亮当前焦点
    if (settingsFocus == 0) {
      display.fillRect(0, 18, 128, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(0, 20);
      display.print("Freq: ");
      display.print(buzzerFreq);
      display.println(" Hz");
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.fillRect(0, 30, 128, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(0, 32);
      display.print("BPM:  ");
      display.print(drumBPM);
      display.setTextColor(SSD1306_WHITE);
    }
    display.display();
    return;
  }

  // ---- 正常界面 ----
  drawModeRow(mode, row);

  display.setTextSize(1);
  if (mode == MODE_TIMER) {
    int h = remainingSeconds / 3600;
    int m = (remainingSeconds % 3600) / 60;
    int s = remainingSeconds % 60;
    display.setTextSize(2);
    bool focusHour = (row == ROW_CONTENT && contentFocus == 0);
    bool focusMin  = (row == ROW_CONTENT && contentFocus == 1);
    bool focusSec  = (row == ROW_CONTENT && contentFocus == 2);
    drawNumberWithFocus(0,  20, h, focusHour);
    display.print(" : ");
    drawNumberWithFocus(40, 20, m, focusMin);
    display.print(" : ");
    drawNumberWithFocus(80, 20, s, focusSec);
    display.setTextSize(1);
    display.setCursor(0, 42);
    const char* stateNames[] = {"IDLE", "RUN", "PAUSE", "ALARM"};
    display.print("State: ");
    display.print(stateNames[(int)timerState]);
  } 
  else if (mode == MODE_DRUM) {
    int startX = 0, startY = 20;
    int cellSize = 7, gap = 1;
    for (int i=0; i<DRUM_STEPS; i++) {
      int x = startX + i * (cellSize + gap);
      int y = startY;
      bool isActive = drumPattern[i][drumEditInstrument];
      bool isFocus = (row == ROW_CONTENT && contentFocus == i);
      if (isFocus) {
        display.fillRect(x-1, y-1, cellSize+2, cellSize+2, SSD1306_WHITE);
        display.fillRect(x, y, cellSize, cellSize, SSD1306_BLACK);
      } else {
        display.fillRect(x, y, cellSize, cellSize, isActive ? SSD1306_WHITE : SSD1306_BLACK);
      }
      if (drumPlaying && i == drumCurrentPlayStep) {
        display.fillTriangle(x+1, y-3, x+cellSize/2, y-1, x+cellSize-1, y-3, SSD1306_WHITE);
      }
    }
    display.setCursor(0, 42);
    if (row == ROW_INSTRUMENT) {
      display.fillRect(0, 40, 128, 10, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    }
    const char* instNames[] = {"Kick", "HiHat", "Light"};
    display.print("Inst: ");
    display.print(instNames[drumEditInstrument]);
    display.setTextColor(SSD1306_WHITE);
  } 
  else if (mode == MODE_WHEEL) {
    display.setTextSize(2);
    display.setCursor(20, 25);
    display.print("Wheel");
    display.setTextSize(1);
    display.setCursor(20, 45);
    if (wheelSpinning) display.print("Spinning...");
    else display.print("Target: ");
    display.print(wheelTarget);
  }

  drawActionRow(row, mode, timerState, drumPlaying);
  display.display();
}