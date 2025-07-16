#include <esp32-hal-ledc.h>
#include "BluetoothSerial.h" // ESP32 내장 블루투스 시리얼 라이브러리 포함

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif

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

// --- BluetoothSerial 객체 생성 ---
BluetoothSerial SerialBT;

// --- 현재 LED 색상 값을 저장할 변수 ---
int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;

// --- 함수 선언 ---
void setColor(int r, int g, int b);

// --- 색상을 설정하는 함수 (공통 음극 LED용: 0=꺼짐, 255=밝음) ---
void setColor(int r, int g, int b) {
  ledcWrite(redChannel, r);
  ledcWrite(greenChannel, g);
  ledcWrite(blueChannel, b);
  currentRed = r;
  currentGreen = g;
  currentBlue = b;
}

void setup() {
  Serial.begin(115200); // USB 시리얼 통신 시작 (디버깅 용도)

  // --- LED PWM 채널 설정 ---
  ledcSetup(redChannel, freq, resolution);
  ledcAttach(ledR, redChannel);

  ledcSetup(greenChannel, freq, resolution);
  ledcAttach(ledG, greenChannel);

  ledcSetup(blueChannel, freq, resolution);
  ledcAttach(ledB, blueChannel);

  // 초기 LED 상태 (꺼짐)
  setColor(0, 0, 0);

  // --- 블루투스 시리얼 시작 ---
  // "ESP32_LED_Control"은 스마트폰에서 검색될 블루투스 장치 이름입니다.
  SerialBT.begin("ESP32_LED_Control");
  Serial.println("Bluetooth Device Started! Ready to pair...");
  Serial.println("Connect to 'ESP32_LED_Control' from your smartphone.");
}

void loop() {
  // --- 블루투스를 통해 데이터가 수신되면 ---
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n'); // 개행 문자까지 읽어옴
    command.trim(); // 앞뒤 공백 제거
    Serial.print("Received command: ");
    Serial.println(command);

    if (command == "RED") {
      setColor(255, 0, 0);
      Serial.println("LED set to RED");
    } else if (command == "GREEN") {
      setColor(0, 255, 0);
      Serial.println("LED set to GREEN");
    } else if (command == "BLUE") {
      setColor(0, 0, 255);
      Serial.println("LED set to BLUE");
    } else if (command == "WHITE") {
      setColor(255, 255, 255);
      Serial.println("LED set to WHITE");
    } else if (command == "OFF") {
      setColor(0, 0, 0);
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
    SerialBT.println("Command received: " + command); // 스마트폰으로 응답
  }

  // --- USB 시리얼을 통해 명령을 받으면 (디버깅용) ---
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.print("USB Received: ");
    Serial.println(command);
    // 블루투스로 받은 것과 동일하게 처리 (테스트 용이)
    if (command == "RED") {
      setColor(255, 0, 0);
      Serial.println("LED set to RED");
    } else if (command == "GREEN") {
      setColor(0, 255, 0);
      Serial.println("LED set to GREEN");
    } else if (command == "BLUE") {
      setColor(0, 0, 255);
      Serial.println("LED set to BLUE");
    } else if (command == "WHITE") {
      setColor(255, 255, 255);
      Serial.println("LED set to WHITE");
    } else if (command == "OFF") {
      setColor(0, 0, 0);
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

  // 다른 작업을 여기에 추가할 수 있습니다.
  delay(10); // 루프 지연
}
