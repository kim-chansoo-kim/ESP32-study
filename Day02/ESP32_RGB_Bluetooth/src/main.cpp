// --- main.cpp ---
#include <Arduino.h> // PlatformIO에서는 이 줄이 필요합니다.
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif
// ESP32의 Serial2 (UART2)를 사용하여 외부 블루투스 모듈과 통신합니다.
// GPIO16 (RX2), GPIO17 (TX2) 핀을 사용합니다.

// --- RGB LED 핀 설정 (KY-016 모듈용) ---
// KY-016은 공통 음극(Common Cathode)이므로 0이 꺼짐, 255가 밝음
uint8_t ledR = 18; // 빨강 LED가 연결된 GPIO 핀 번호
uint8_t ledG = 19; // 초록 LED가 연결된 GPIO 핀 번호
uint8_t ledB = 21; // 파랑 LED가 연결된 GPIO 핀 번호

// --- LED PWM 채널 설정 ---
const int freq = 5000;    // PWM 주파수 (Hz)
const int resolution = 8; // PWM 해상도 (비트). 8비트는 0~255 값 사용
const int redChannel = 0; // 빨강 LED에 할당할 PWM 채널
const int greenChannel = 1; // 초록 LED에 할당할 PWM 채널
const int blueChannel = 2;  // 파랑 LED에 할당할 PWM 채널

// --- 함수 선언 ---
void setColor(int r, int g, int b);

// --- 색상을 설정하는 함수 (공통 음극 LED용: 0=꺼짐, 255=밝음) ---
void setColor(int r, int g, int b) {
  ledcWrite(redChannel, r);
  ledcWrite(greenChannel, g);
  ledcWrite(blueChannel, b);
}

void setup() {
  Serial.begin(115200); // USB 시리얼 통신 시작 (컴퓨터 시리얼 모니터용)

  // --- LED PWM 채널 설정 ---
  ledcSetup(redChannel, freq, resolution);
  ledcAttachPin(ledR, redChannel);

  ledcSetup(greenChannel, freq, resolution);
  ledcAttachPin(ledG, greenChannel);

  ledcSetup(blueChannel, freq, resolution);
  ledcAttachPin(ledB, blueChannel);

  // 초기 LED 상태 (꺼짐)
  setColor(0, 0, 0);

  // --- 외부 블루투스 모듈 (Serial2) 시작 ---
  // RX2 (GPIO16), TX2 (GPIO17) 핀을 사용하여 Serial2 통신을 시작합니다.
  // 대부분의 HC-05/06 모듈은 기본 9600 보드레이트를 사용합니다.
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Baud rate, Data bits, Parity, Stop bits, RX pin, TX pin
  Serial.println("External Bluetooth Module Started! Ready for commands...");
  Serial.println("Connect to your Bluetooth module (e.g., HC-05/06) from your smartphone.");
}

void loop() {
  // --- 외부 블루투스 모듈을 통해 데이터가 수신되면 ---
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n'); // 개행 문자까지 읽어옴
    command.trim(); // 앞뒤 공백 제거
    Serial.print("Received Bluetooth command: "); // USB 시리얼 모니터에 출력
    Serial.println(command);

    if (command == "RED") {
      setColor(0, 255, 255);
      Serial.println("LED set to RED");
    } else if (command == "GREEN") {
      setColor(255, 0, 255);
      Serial.println("LED set to GREEN");
    } else if (command == "BLUE") {
      setColor(255, 255, 0);
      Serial.println("LED set to BLUE");
    } else if (command == "WHITE") {
      setColor(0, 0, 0);
      Serial.println("LED set to WHITE");
    } else if (command == "OFF") {
      setColor(255, 255, 255);
      Serial.println("LED set to OFF");
    } else if (command.startsWith("RGB")) {
      // "RGB(r,g,b)" 형식의 명령 처리
      int r = 0, g = 0, b = 0;
      sscanf(command.c_str(), "RGB(%d,%d,%d)", &r, &g, &b);
      // 값 유효성 검사 (0~255 범위)
      r = constrain(r, 0, 255);
      g = constrain(g, 0, 255);
      b = constrain(b, 0, 255);
      setColor(r, g, b);
      Serial.printf("LED set to RGB(%d,%d,%d)\n", r, g, b);
    } else {
      Serial.println("Unknown command.");
    }
    Serial2.println("Command received: " + command); // 블루투스 모듈로 응답
  }

  // --- USB 시리얼을 통해 명령을 받으면 (디버깅용, 컴퓨터에서 직접 명령 입력) ---
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.print("USB Received: ");
    Serial.println(command);
    // 블루투스로 받은 것과 동일하게 처리 (테스트 용이)
    if (command == "RED") {
      setColor(0, 255, 255);
      Serial.println("LED set to RED");
    } else if (command == "GREEN") {
      setColor(255, 0, 255);
      Serial.println("LED set to GREEN");
    } else if (command == "BLUE") {
      setColor(255, 255, 0);
      Serial.println("LED set to BLUE");
    } else if (command == "WHITE") {
      setColor(0, 0, 0);
      Serial.println("LED set to WHITE");
    } else if (command == "OFF") {
      setColor(255, 255, 255);
      Serial.println("LED set to OFF");
    } else if (command.startsWith("RGB")) {
      int r = 0, g = 0, b = 0;
      sscanf(command.c_str(), "RGB(%d,%d,%d)", &r, &g, &b);
      r = constrain(r, 0, 255);
      g = constrain(g, 0, 255);
      b = constrain(b, 0, 255);
      setColor(r, g, b);
      Serial.printf("LED set to RGB(%d,%d,%d)\n", r, g, b);
    } else {
      Serial.println("Unknown command.");
    }
  }
  delay(10); // 루프 지연
}
