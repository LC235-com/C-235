#ifndef CONFIG_H
#define CONFIG_H

// ==================== 步进电机引脚（ULN2003） ====================
#define MOTOR_PIN1  26
#define MOTOR_PIN2  25
#define MOTOR_PIN3  33
#define MOTOR_PIN4  32

// ==================== 按钮引脚（低电平有效，上拉输入） ====================
#define BTN_OK      36   // 按钮1：确认/切换
#define BTN_INC     39   // 按钮2：左/减小
#define BTN_DEC     34   // 按钮3：右/增大
#define BTN_MOVE    35   // 按钮4：上下切换行

// ==================== OLED（I2C） ====================
#define OLED_SDA    23   // 通常 I2C 默认，但可自定义
#define OLED_SCL    22
#define OLED_ADDR   0x3C // 常见地址，若不同请修改

// ==================== 流水灯 ====================
#define LED_STRIP_PIN  4

// ==================== 蜂鸣器 ====================
#define BUZZER_PIN  2

// ==================== 倒计时参数 ====================
#define MAX_SECONDS 7200   // 最大2小时
#define STEP_PER_REV 2048  // 28BYJ-48 半步模式步数
#define FLIP_INTERVAL 1   // 每分钟翻转一次（秒）
// ==================== 鼓机参数 ====================
#define DRUM_STEPS 16
#define DRUM_INSTRUMENTS 3  // 0=Kick(翻牌器), 1=HiHat(蜂鸣器), 2=Light(流水灯)

#endif