/*
 这是用Adafruit_SSD1306库来操作OLED显示屏的代码，注释掉了

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH      128   // OLED display width, in pixels
#define SCREEN_HEIGHT     64    // OLED display height, in pixels
#define OLED_RESET        -1    // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS    0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// 构造对象 连接到I2C(SDA、SCL引脚)的SSD1306声明
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // OLED初始化
  OLED.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  // OLED清除显示
  OLED.clearDisplay();
  // OLED设置光标位置
  OLED.setCursor(1, 1);
  // 设置字体颜色
  OLED.setTextColor(SSD1306_WHITE);
  // 显示字符串内容
  OLED.println("Good Good Study!");
  OLED.println("Day Day Up!");

  OLED.display();
}

void loop() {
  // put your main code here, to run repeatedly:
}
*/
/*
这是用U8g2库来操作OLED显示屏的代码
*/
#include <Arduino.h>
#include <U8g2lib.h>

// 自定义CLK和SDA引脚
#define OLED_CLK  18  
#define OLED_SDA  5
// 构造对象
U8G2_SSD1306_128X64_NONAME_F_SW_I2C OLED(U8G2_R0, OLED_CLK, OLED_SDA, U8X8_PIN_NONE);

void setup() {
  // 初始化OLED
  OLED.begin();
  // 开启中文字符集支持
  OLED.enableUTF8Print();
  // 设置字体
  OLED.setFont(u8g2_font_wqy12_t_gb2312);
}

void loop() {
 
  // 清除缓存区内容
  OLED.clearBuffer();
  // 绘制内容
  OLED.setCursor(0, 10);
  OLED.print("好好学习,");

  OLED.setCursor(0, 30);
  OLED.print("天天向上!");

  // 发送缓存区内容到OLED
  OLED.sendBuffer();

  delay(1000);
}

