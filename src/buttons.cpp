#include "buttons.h"
#include "config.h"

static ButtonCallback okCB = nullptr;
static ButtonCallback incCB = nullptr;
static ButtonCallback decCB = nullptr;
static ButtonCallback moveCB = nullptr;

struct ButtonState {
  uint8_t pin;
  bool lastState;
  unsigned long lastDebounceTime;
  bool pressed;
};

static ButtonState btnStates[] = {
  {BTN_OK,   HIGH, 0, false},
  {BTN_INC,  HIGH, 0, false},
  {BTN_DEC,  HIGH, 0, false},
  {BTN_MOVE, HIGH, 0, false}
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
        switch(i) {
          case 0: if(okCB) okCB(); break;
          case 1: if(incCB) incCB(); break;
          case 2: if(decCB) decCB(); break;
          case 3: if(moveCB) moveCB(); break;
        }
      } else if (reading == HIGH) {
        btnStates[i].pressed = false;
      }
    }
    btnStates[i].lastState = reading;
  }
}

void setOkCallback(ButtonCallback cb) { okCB = cb; }
void setIncCallback(ButtonCallback cb) { incCB = cb; }
void setDecCallback(ButtonCallback cb) { decCB = cb; }
void setMoveCallback(ButtonCallback cb) { moveCB = cb; }