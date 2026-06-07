#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== OLED 配置 ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// 如果你使用默认 I2C 引脚 (SDA=21, SCL=22)，请注释下面两行
// 如果你使用自定义引脚 (例如 SDA=5, SCL=18)，请取消注释并修改
// #define OLED_SDA 5
// #define OLED_SCL 18

#ifdef OLED_SDA
  TwoWire I2C_OLED = TwoWire(0);
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_OLED, OLED_RESET);
#else
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// ========== WiFi 配置 ==========
const char* ssid     = "LC235的场域";
const char* password = "ub7nqripvyaaxkc";

// ========== WebSocket 配置 ==========
const char* ws_host  = "192.168.49.4";   // 改成电脑的 IP 地址
const uint16_t ws_port = 8765;

WebSocketsClient webSocket;

// ========== 状态变量 ==========
unsigned long lastHeartbeatSend = 0;
const unsigned long heartbeatInterval = 25000; // 25秒发一次 ping
bool wsConnected = false;
int heartbeatOK = 0;    // 收到 pong 的次数
int heartbeatFail = 0;  // 未收到 pong 的次数（可选）

// ========== OLED 显示更新（非阻塞） ==========
unsigned long lastDisplayUpdate = 0;
String wifiStatus = "Connecting...";
String wsStatus = "Disconnected";
String lastPongTime = "--";

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("WiFi: " + wifiStatus);
  display.setCursor(0, 10);
  display.println("WebSocket: " + wsStatus);
  display.setCursor(0, 20);
  display.println("Heartbeat Pong: " + String(heartbeatOK));
  display.setCursor(0, 30);
  display.println("Last Pong: " + lastPongTime);
  display.setCursor(0, 45);
  display.println("IP: " + WiFi.localIP().toString());
  display.display();
}

// ========== WebSocket 事件回调 ==========
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      wsConnected = false;
      wsStatus = "Disconnected";
      Serial.println("[WS] Disconnected");
      updateOLED();
      break;
    case WStype_CONNECTED:
      wsConnected = true;
      wsStatus = "Connected";
      Serial.println("[WS] Connected");
      updateOLED();
      break;
    case WStype_TEXT:
      // 处理服务器返回的 pong 或其他文本消息
      if (strcmp((char*)payload, "pong") == 0) {
        heartbeatOK++;
        lastPongTime = String(millis() / 1000);
        Serial.println("[WS] Received pong");
        updateOLED();
      } else {
        Serial.printf("[WS] Received text: %s\n", payload);
      }
      break;
    case WStype_BIN:
      // 接收画面数据（可选，若不需要可以注释掉）
      if (length == 1024) {
        display.clearDisplay();
        display.drawBitmap(0, 0, payload, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
        display.display();
      }
      break;
    case WStype_ERROR:
      Serial.println("[WS] Error");
      wsStatus = "Error";
      updateOLED();
      break;
    default:
      break;
  }
}

// ========== WiFi 连接 ==========
void connectWiFi() {
  WiFi.begin(ssid, password);
  wifiStatus = "Connecting...";
  updateOLED();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  wifiStatus = "Connected";
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  updateOLED();
}

// ========== 设置 ==========
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // 初始化 I2C 总线（自定义引脚时需要）
  #ifdef OLED_SDA
    I2C_OLED.begin(OLED_SDA, OLED_SCL, 400000);
  #endif

  // 初始化 OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 如果地址是 0x3D 请修改
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Booting...");
  display.display();

  // 连接 WiFi
  connectWiFi();

  // 配置 WebSocket 客户端
  webSocket.begin(ws_host, ws_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);   // 断线重连间隔 5 秒
  webSocket.enableHeartbeat(15000, 3000, 2); // 底层心跳（可选，与我们的应用层心跳互补）
}

// ========== 主循环 ==========
void loop() {
  webSocket.loop();  // 必须循环调用

  // 应用层心跳：定期发送 "ping"
  if (wsConnected && (millis() - lastHeartbeatSend > heartbeatInterval)) {
    webSocket.sendTXT("ping");
    lastHeartbeatSend = millis();
    Serial.println("[WS] Sent ping");
  }

  // 每 2 秒刷新一次 OLED 显示（如果状态变化频繁可提高频率）
  if (millis() - lastDisplayUpdate > 100000) {
    lastDisplayUpdate = millis();
    updateOLED();
  }
}