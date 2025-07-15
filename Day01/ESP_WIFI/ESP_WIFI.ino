#include <WiFi.h> // Wi-Fi 라이브러리 포함

const char* ssid = "PKNU_58";     // 접속할 Wi-Fi 이름
const char* password = "iotiot58"; // Wi-Fi 비밀번호

void setup() {
  Serial.begin(115200);
  delay(10); // 잠시 대기

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // Wi-Fi 연결 시도

  while (WiFi.status() != WL_CONNECTED) { // 연결될 때까지 대기
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // 할당받은 IP 주소 출력
}

void loop() {
  // Wi-Fi 연결 후 할 작업을 여기에 추가 (예: 서버 연결, 데이터 전송 등)
}