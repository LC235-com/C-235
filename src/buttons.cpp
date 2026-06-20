#include "buttons.h"
#include "config.h"

static ButtonCallback moveCB = nullptr;
static ButtonCallback incCB = nullptr;
static ButtonCallback decCB = nullptr;
static ButtonCallback okCB = nullptr;

// 按键状态
static struct {
  uint8_t pin;
  bool lastState;
  unsigned long lastDebounceTime;
  bool pressed;
} btnStates[] = {
  {BTN_MOVE, HIGH, 0, false},
  {BTN_INC,  HIGH, 0, false},
  {BTN_DEC,  HIGH, 0, false},
  {BTN_OK,   HIGH, 0, false}
};

const unsigned long DEBOUNCE_DELAY = 50;

void initButtons() {
  for (int i=0; i<4; i++) {
    pinMode(btnStates[i].pin, INPUT_PULLUP);
    btnStates[i].lastState = digitalRead(btnStates[i].pin);
  }
}

void scanButtons() {
  unsigned long now = millis();
  for (int i=0; i<4; i++) {
    bool reading = digitalRead(btnStates[i].pin);
    if (reading != btnStates[i].lastState) {
      btnStates[i].lastDebounceTime = now;
    }
    if ((now - btnStates[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading == LOW && !btnStates[i].pressed) {
        btnStates[i].pressed = true;
        // 触发回调
        switch(i) {
          case 0: if(moveCB) moveCB(); break;
          case 1: if(incCB) incCB(); break;
          case 2: if(decCB) decCB(); break;
          case 3: if(okCB) okCB(); break;
        }
      } else if (reading == HIGH) {
        btnStates[i].pressed = false;
      }
    }
    btnStates[i].lastState = reading;
  }
}

void setMoveCallback(ButtonCallback cb) { moveCB = cb; }
void setIncCallback(ButtonCallback cb) { incCB = cb; }
void setDecCallback(ButtonCallback cb) { decCB = cb; }
void setOkCallback(ButtonCallback cb) { okCB = cb; }