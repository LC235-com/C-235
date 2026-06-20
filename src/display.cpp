#include "display.h"
#include "config.h"
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

// 绘制带焦点的高亮数字（反色框）
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

void drawUI(int remainingSeconds, Focus focus, ActionType action, TimerState state) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  // 标题和状态
  const char* stateStr[] = {"IDLE", "RUN", "PAUSE", "ALARM"};
  display.print("Timer [");
  display.print(stateStr[state]);
  display.println("]");

  // 时间分解
  int h = remainingSeconds / 3600;
  int m = (remainingSeconds % 3600) / 60;
  int s = remainingSeconds % 60;

  // 绘制时间
  display.setTextSize(2);
  drawNumberWithFocus(0,  20, h, focus == FOCUS_HOUR);
  display.print(" : ");
  drawNumberWithFocus(40, 20, m, focus == FOCUS_MIN);
  display.print(" : ");
  drawNumberWithFocus(80, 20, s, focus == FOCUS_SEC);

  // 绘制操作按钮行
  display.setTextSize(1);
  display.setCursor(0, 48);
  // 根据状态决定可用操作，但为了简单，我们固定显示三个，并用焦点高亮当前选中的
  const char* actionNames[] = {"Start", "Pause", "Reset"};
  int actionIdx = (int)action;
  for (int i=0; i<3; i++) {
    if (i == actionIdx && focus == FOCUS_ACTION) {
      display.fillRect(i*42-2, 46, 40, 14, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(i*42, 48);
      display.print(actionNames[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(i*42, 48);
      display.print(actionNames[i]);
    }
  }
  display.display();
}