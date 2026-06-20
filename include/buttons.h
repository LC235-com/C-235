#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// 定义回调函数类型
typedef void (*ButtonCallback)(void);

void initButtons();
void scanButtons(); // 在loop中调用
void setMoveCallback(ButtonCallback cb);
void setIncCallback(ButtonCallback cb);
void setDecCallback(ButtonCallback cb);
void setOkCallback(ButtonCallback cb);

#endif