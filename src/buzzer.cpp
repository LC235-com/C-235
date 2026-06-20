#include "buzzer.h"
#include "config.h"

#define BUZZER_CHANNEL 0

static bool alarmActive = false;
static unsigned long alarmToggleTime = 0;
static bool alarmState = false;

static bool beepActive = false;
static unsigned long beepStartTime = 0;
static int beepDuration = 0;

void initBuzzer() {
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);
}

void startAlarm() {
  alarmActive = true;
  alarmToggleTime = millis();
  alarmState = true;
  ledcWrite(BUZZER_CHANNEL, 128);
}

void stopAlarm() {
  alarmActive = false;
  ledcWrite(BUZZER_CHANNEL, 0);
}

bool isAlarmActive() {
  return alarmActive;
}

void playBeep(int freq, int durationMs) {
  if (beepActive) return;
  beepActive = true;
  beepStartTime = millis();
  beepDuration = durationMs;
  ledcSetup(BUZZER_CHANNEL, freq, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 128);
}

void updateBuzzer() {
  if (beepActive) {
    if (millis() - beepStartTime >= beepDuration) {
      ledcWrite(BUZZER_CHANNEL, 0);
      beepActive = false;
      if (alarmActive) {
        ledcWrite(BUZZER_CHANNEL, 128);
      }
    }
    return;
  }
  if (alarmActive) {
    unsigned long now = millis();
    if (now - alarmToggleTime >= 200) {
      alarmToggleTime = now;
      alarmState = !alarmState;
      ledcWrite(BUZZER_CHANNEL, alarmState ? 128 : 0);
    }
  }
}