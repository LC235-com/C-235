#include <Arduino.h>
#include "config.h"
#include "motor.h"
#include "display.h"
#include "buttons.h"
#include "buzzer.h"

StepperMotor motor(MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4, STEP_PER_REV);

// ========== 模式与焦点 ==========
AppMode currentMode = MODE_TIMER;
RowFocus currentRow = ROW_CONTENT;
int contentFocus = 0;  // 计时：0时1分2秒；鼓机：0~15步

// ========== 倒计时 ==========
int setSeconds = 600;
int remainingSeconds = 600;
TimerState timerState = STATE_IDLE;
bool timerRunning = false;
unsigned long lastTimerTick = 0;

// ========== 鼓机 ==========
bool drumPattern[DRUM_STEPS][DRUM_INSTRUMENTS];
int drumEditInstrument = 0;
int drumCurrentPlayStep = 0;
bool drumPlaying = false;
unsigned long drumNextStepTime = 0;
int drumBPM = 120;          // 可调，全局使用

// 预设节奏（Kick 稀疏，HiHat 反拍，Light 随 Kick）
const bool presetRock[DRUM_STEPS][DRUM_INSTRUMENTS] = {
  {1,0,1}, // 0
  {0,1,0}, // 1
  {0,0,0}, // 2
  {0,1,0}, // 3
  {1,0,1}, // 4
  {0,1,0}, // 5
  {0,0,0}, // 6
  {0,1,0}, // 7
  {1,0,1}, // 8
  {0,1,0}, // 9
  {0,0,0}, // 10
  {0,1,0}, // 11
  {1,0,1}, // 12
  {0,1,0}, // 13
  {0,0,0}, // 14
  {0,1,0}  // 15
};

// ========== 转盘 ==========
int wheelTarget = 0;
bool wheelSpinning = false;
int wheelTotalSteps = 0;

// ========== 流水灯 ==========
unsigned long lastLedToggle = 0;
bool ledFlashState = false;
unsigned long ledFlashInterval = 200;
bool ledFlashing = false;

// ========== 设置页面 ==========
bool inSettings = false;
int settingsFocus = 0;       // 0=频率, 1=BPM
int buzzerFreq = 2000;       // 可调，全局使用

// ========== 函数声明 ==========
void updateTimer();
void updateDrum();
void updateWheel();
void updateLed();
void drawUI();
void switchMode(bool forward);

void handleOk();
void handleInc();
void handleDec();
void handleMove();
void executeAction();
void startWheelSpin();

// ========== setup ==========
void setup() {
  Serial.begin(115200);
  initDisplay();
  motor.init();
  motor.setSpeed(3);
  initButtons();
  initBuzzer();
  pinMode(LED_STRIP_PIN, OUTPUT);
  digitalWrite(LED_STRIP_PIN, HIGH); // 初始关闭（高电平停止流水）

  memcpy(drumPattern, presetRock, sizeof(presetRock));

  setOkCallback(handleOk);
  setIncCallback(handleInc);
  setDecCallback(handleDec);
  setMoveCallback(handleMove);

  drawUI();
  // ---- 测试流水灯 ----
  pinMode(LED_STRIP_PIN, OUTPUT);
  digitalWrite(LED_STRIP_PIN, HIGH); // 先停止
  delay(1000);
  digitalWrite(LED_STRIP_PIN, LOW);  // 低电平应流水
  delay(2000);
  // 然后进入快速闪烁测试（5秒）
  for (int i=0; i<25; i++) {
    digitalWrite(LED_STRIP_PIN, HIGH);
    delay(100);
    digitalWrite(LED_STRIP_PIN, LOW);
    delay(100);
  }
  // 测试结束后恢复默认（低电平流水）
  digitalWrite(LED_STRIP_PIN, LOW);
}

// ========== loop ==========
void loop() {
  scanButtons();
  motor.update();
  updateBuzzer();
  updateLed();

  if (!inSettings) {
    switch(currentMode) {
      case MODE_TIMER: updateTimer(); break;
      case MODE_DRUM:  updateDrum();  break;
      case MODE_WHEEL: updateWheel(); break;
    }
  }
}

// ========== 更新函数 ==========
void updateTimer() {
  if (timerRunning && remainingSeconds > 0) {
    unsigned long now = millis();
    if (now - lastTimerTick >= 1000) {
      lastTimerTick = now;
      remainingSeconds--;
      // 每秒翻牌一次
      if (!motor.isBusy()) {
        motor.startRotate(STEP_PER_REV / 10, false); // 修正方向
      }
      if (remainingSeconds == 0) {
        timerRunning = false;
        timerState = STATE_ALARM;
        startAlarm();
      }
      drawUI();
    }
  }
}

void updateDrum() {
  if (!drumPlaying) return;
  unsigned long now = millis();
  unsigned long stepInterval = 60000 / (drumBPM * 4); // 16分音符
  if (now - drumNextStepTime >= stepInterval) {
    drumNextStepTime = now;
    // Kick
    if (drumPattern[drumCurrentPlayStep][0]) {
      if (!motor.isBusy()) {
        motor.startRotate(STEP_PER_REV / 10, false);
      }
    }
    // HiHat
    if (drumPattern[drumCurrentPlayStep][1]) {
      playBeep(buzzerFreq, 30);
    }
    // Light 在 updateLed 中根据 drumPattern[drumCurrentPlayStep][2] 处理
    drumCurrentPlayStep = (drumCurrentPlayStep + 1) % DRUM_STEPS;
    drawUI();
  }
}

void updateWheel() {
  if (wheelSpinning) {
    int remaining = motor.getRemainingSteps();
    if (remaining > 0) {
      // 将剩余步数映射到闪烁间隔：从 50ms（快）到 1000ms（慢）
      unsigned long interval = map(remaining, 0, wheelTotalSteps, 1000, 50);
      ledFlashInterval = interval;
    } else {
      // 电机停止，但需确认电机真正空闲
      if (!motor.isBusy()) {
        wheelSpinning = false;
        int pitch = 500 + wheelTarget * 100;
        playBeep(pitch, 300);
        // 关闭流水灯
        ledFlashing = false;
        digitalWrite(LED_STRIP_PIN, HIGH);
        drawUI();
      }
    }
  }
}

void updateLed() {
  if (inSettings) return;  // 设置页面不干扰

  unsigned long now = millis();
  bool output = HIGH;      // 默认停止（高电平）
  bool flash = false;      // 是否闪烁
  unsigned long interval = 200; // 闪烁间隔

  if (currentMode == MODE_TIMER) {
    if (timerState == STATE_ALARM) {
      // 报警：高频闪烁（间隔200ms）
      flash = true;
      interval = 200;
    } else {
      // 正常计时：持续流水（低电平）
      flash = false;
      output = LOW;
    }
  } 
  else if (currentMode == MODE_DRUM) {
    // 鼓机：不闪烁，仅控制流水/停止
    if (drumPlaying) {
      if (drumPattern[drumCurrentPlayStep][2]) {
        // Light 激活：流水（低电平）
        flash = false;
        output = LOW;
      } else {
        flash = false;
        output = HIGH; // 停止
      }
    } else {
      flash = false;
      output = HIGH; // 未播放时停止
    }
  } 
  else if (currentMode == MODE_WHEEL) {
    if (wheelSpinning) {
      // 转盘旋转：闪烁（频率由 updateWheel 动态调整）
      flash = true;
      interval = ledFlashInterval; // 使用动态值
    } else {
      flash = false;
      output = HIGH; // 停止
    }
  }

  // 执行输出
  if (flash) {
    if (now - lastLedToggle >= interval) {
      lastLedToggle = now;
      ledFlashState = !ledFlashState;
      // 低电平亮（流水），高电平灭（停止）
      digitalWrite(LED_STRIP_PIN, ledFlashState ? LOW : HIGH);
    }
  } else {
    digitalWrite(LED_STRIP_PIN, output);
    ledFlashState = false; // 重置
  }
}

// ========== 绘图 ==========
void drawUI() {
  ::drawUI(currentMode, currentRow, contentFocus, remainingSeconds, timerState,
           drumPlaying, drumCurrentPlayStep, drumPattern, drumEditInstrument, wheelTarget, wheelSpinning);
}

// ========== 模式切换 ==========
void switchMode(bool forward) {
  if (forward) {
    if (currentMode == MODE_TIMER) currentMode = MODE_DRUM;
    else if (currentMode == MODE_DRUM) currentMode = MODE_WHEEL;
    else currentMode = MODE_TIMER;
  } else {
    if (currentMode == MODE_TIMER) currentMode = MODE_WHEEL;
    else if (currentMode == MODE_WHEEL) currentMode = MODE_DRUM;
    else currentMode = MODE_TIMER;
  }
  // 停止所有播放
  if (timerRunning) { timerRunning = false; timerState = STATE_IDLE; }
  if (drumPlaying) { drumPlaying = false; }
  if (wheelSpinning) { wheelSpinning = false; motor.startRotate(0, true); }
  stopAlarm();
  contentFocus = 0;
  ledFlashing = false;
  digitalWrite(LED_STRIP_PIN, HIGH);
  drawUI();
}

// ========== 按键回调 ==========
void handleOk() {
  if (inSettings) {
    inSettings = false;
    currentRow = ROW_INSTRUMENT;
    drawUI();
    return;
  }

  if (currentRow == ROW_MODE) {
    // 模式行无操作
    return;
  } 
  else if (currentRow == ROW_CONTENT) {
    if (currentMode == MODE_DRUM) {
      if (contentFocus < DRUM_STEPS) {
        drumPattern[contentFocus][drumEditInstrument] = !drumPattern[contentFocus][drumEditInstrument];
        drawUI();
      }
    } else if (currentMode == MODE_TIMER) {
      currentRow = ROW_ACTION;
      drawUI();
    } else if (currentMode == MODE_WHEEL) {
      if (!wheelSpinning && !motor.isBusy()) {
        startWheelSpin();
      }
    }
  } 
  else if (currentRow == ROW_INSTRUMENT) {
    if (currentMode == MODE_DRUM) {
      inSettings = true;
      settingsFocus = 0;
      drawUI();
      return;
    }
  } 
  else if (currentRow == ROW_ACTION) {
    executeAction();
  }
}

void handleInc() { // 左/减小
  if (inSettings) {
    if (settingsFocus == 0) {
      buzzerFreq -= 20;
      if (buzzerFreq < 200) buzzerFreq = 200;
    } else if (settingsFocus == 1) {
      drumBPM -= 5;
      if (drumBPM < 40) drumBPM = 40;
    }
    drawUI();
    return;
  }

  if (currentRow == ROW_MODE) {
    switchMode(false);
    drawUI();
    return;
  }

  if (currentRow == ROW_CONTENT) {
    if (currentMode == MODE_TIMER) {
      int delta = -1;
      if (contentFocus == 0) delta = -3600;
      else if (contentFocus == 1) delta = -60;
      else delta = -1;
      setSeconds += delta;
      if (setSeconds < 0) setSeconds = 0;
      if (setSeconds > MAX_SECONDS) setSeconds = MAX_SECONDS;
      remainingSeconds = setSeconds;
      drawUI();
    } else if (currentMode == MODE_DRUM) {
      if (contentFocus > 0) contentFocus--;
      else contentFocus = DRUM_STEPS - 1;
      drawUI();
    }
  } else if (currentRow == ROW_INSTRUMENT) {
    if (currentMode == MODE_DRUM) {
      drumEditInstrument = (drumEditInstrument - 1 + DRUM_INSTRUMENTS) % DRUM_INSTRUMENTS;
      drawUI();
    }
  }
}

void handleDec() { // 右/增大
  if (inSettings) {
    if (settingsFocus == 0) {
      buzzerFreq += 20;
      if (buzzerFreq > 5000) buzzerFreq = 5000;
    } else if (settingsFocus == 1) {
      drumBPM += 5;
      if (drumBPM > 200) drumBPM = 200;
    }
    drawUI();
    return;
  }

  if (currentRow == ROW_MODE) {
    switchMode(true);
    drawUI();
    return;
  }

  if (currentRow == ROW_CONTENT) {
    if (currentMode == MODE_TIMER) {
      int delta = 1;
      if (contentFocus == 0) delta = 3600;
      else if (contentFocus == 1) delta = 60;
      else delta = 1;
      setSeconds += delta;
      if (setSeconds < 0) setSeconds = 0;
      if (setSeconds > MAX_SECONDS) setSeconds = MAX_SECONDS;
      remainingSeconds = setSeconds;
      drawUI();
    } else if (currentMode == MODE_DRUM) {
      if (contentFocus < DRUM_STEPS - 1) contentFocus++;
      else contentFocus = 0;
      drawUI();
    }
  } else if (currentRow == ROW_INSTRUMENT) {
    if (currentMode == MODE_DRUM) {
      drumEditInstrument = (drumEditInstrument + 1) % DRUM_INSTRUMENTS;
      drawUI();
    }
  }
}

void handleMove() {
  if (inSettings) {
    settingsFocus = (settingsFocus + 1) % 2;
    drawUI();
    return;
  }

  if (currentRow == ROW_MODE) {
    currentRow = ROW_CONTENT;
  } 
  else if (currentRow == ROW_CONTENT) {
    if (currentMode == MODE_TIMER) {
      // 计时模式：循环切换时/分/秒
      contentFocus = (contentFocus + 1) % 3;
    } else if (currentMode == MODE_DRUM) {
      currentRow = ROW_INSTRUMENT;
    } else if (currentMode == MODE_WHEEL) {
      currentRow = ROW_ACTION;
    }
  } 
  else if (currentRow == ROW_INSTRUMENT) {
    currentRow = ROW_ACTION;
  } 
  else if (currentRow == ROW_ACTION) {
    currentRow = ROW_MODE;
  }
  drawUI();
}

// ========== 执行操作 ==========
void executeAction() {
  if (currentMode == MODE_TIMER) {
    if (timerState == STATE_RUNNING) {
      timerRunning = false;
      timerState = STATE_PAUSED;
    } else if (timerState == STATE_PAUSED || timerState == STATE_IDLE) {
      if (remainingSeconds > 0) {
        timerRunning = true;
        timerState = STATE_RUNNING;
        lastTimerTick = millis();
        // 启动时立即翻一次牌？
      }
    } else if (timerState == STATE_ALARM) {
      stopAlarm();
      timerState = STATE_IDLE;
      remainingSeconds = setSeconds;
      timerRunning = false;
      digitalWrite(LED_STRIP_PIN, LOW); // 恢复流水
    }
    drawUI();
  } 
  else if (currentMode == MODE_DRUM) {
    drumPlaying = !drumPlaying;
    if (drumPlaying) {
      drumCurrentPlayStep = 0;
      drumNextStepTime = millis();
    } else {
      digitalWrite(LED_STRIP_PIN, HIGH); // 关闭流水灯
    }
    drawUI();
  } 
  else if (currentMode == MODE_WHEEL) {
    if (!wheelSpinning && !motor.isBusy()) {
      startWheelSpin();
    }
  }
}

// ========== 转盘启动 ==========
void startWheelSpin() {
  wheelTarget = random(0, 10);
  wheelTotalSteps = (5 * 10 + wheelTarget) * (STEP_PER_REV / 10);
  motor.startRotate(wheelTotalSteps, true);
  wheelSpinning = true;
  ledFlashInterval = 50;
  drawUI();
}