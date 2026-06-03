#include <Arduino.h>
int light1 = 1;
int light2 = 1;
void setup() {
    Serial.begin(115200);
    pinMode(2 , OUTPUT);
    pinMode(4 , OUTPUT);
    Serial.println("发送数字点亮对应LED，再次发送熄灭LED");
}

void loop() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '1') {
            digitalWrite(2 , light1);
            if(light1){
                Serial.println("LED1已点亮");
            } else {
                Serial.println("LED1已熄灭");
            }
            if(light1 == 0){
                light1 = HIGH;
            } else if(light1 == 1){
                light1 = LOW;
            }
        } else if (cmd == '2') {
            digitalWrite(4 , light2);
            if(light2){
                Serial.println("LED2已点亮");
            } else {
                Serial.println("LED2已熄灭");
            }
            if(light2 == 0){
                light2 = HIGH;
            } else if(light2 == 1){
                light2 = LOW;
            }    
        } else {
            Serial.println("无效指令，请发送 '1' 或 '2'");
        }
    }
}