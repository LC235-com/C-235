#include "buzzer.h"
#include "config.h"

#define BUZZER_CHANNEL 0   // LEDC通道
#define ALARM_INTERVAL 200 // 蜂鸣间隔（ms）

static bool alarmOn = false;
static unsigned long lastToggle = 0;
static bool state = false;

void initBuzzer() {
  ledcSetup(BUZZER_CHANNEL, 2000, 8); // 2kHz, 8位分辨率
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
}

void startAlarm() {
  alarmOn = true;
  lastToggle = millis();
  state = true;
  ledcWrite(BUZZER_CHANNEL, 128); // 50%占空比
}

void stopAlarm() {
  alarmOn = false;
  ledcWrite(BUZZER_CHANNEL, 0);
}

bool isAlarmActive() {
  return alarmOn;
}

void updateBuzzer() {
  if (!alarmOn) return;
  unsigned long now = millis();
  if (now - lastToggle >= ALARM_INTERVAL) {
    lastToggle = now;
    state = !state;
    if (state) ledcWrite(BUZZER_CHANNEL, 128);
    else ledcWrite(BUZZER_CHANNEL, 0);
  }
}