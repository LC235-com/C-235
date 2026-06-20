#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

void initBuzzer();
void startAlarm();         // 开始闹铃（非阻塞）
void stopAlarm();
bool isAlarmActive();
void updateBuzzer();       // 在loop中调用，控制蜂鸣器发声模式

#endif