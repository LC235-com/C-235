#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

void initBuzzer();
void startAlarm();
void stopAlarm();
bool isAlarmActive();
void playBeep(int freq, int durationMs);
void updateBuzzer();

#endif