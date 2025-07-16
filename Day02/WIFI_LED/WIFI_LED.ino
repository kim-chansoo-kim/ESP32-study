#include <WiFi.h>
#include <WebServer.h> // 웹 서버 라이브러리 포함

// --- Wi-Fi 설정 ---
const char* ssid = "PKNU_58";      // 자신의 Wi-Fi 이름
const char* password = "iotiot58"; // 자신의 Wi-Fi 비밀번호

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

// --- 웹 서버 객체 생성 (기본 포트 80) ---
WebServer server(80);

// --- 현재 LED 색상 값을 저장할 변수 ---
int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;

// --- 색상 변환을 위한 변수 ---
uint8_t colorHue = 0; // 0 to 255 representing the hue
uint32_t R_val, G_val, B_val; // the Red Green and Blue color components
uint8_t brightness_val = 255; // 255 is maximum brightness

// KY-016은 공통 음극이므로 invert는 false로 설정
const boolean invert = false; // set true if common anode, false if common cathode

// --- 함수 선언 ---
void setColor(int r, int g, int b);
void hueToRGB(uint8_t hue, uint8_t brightness);
void handleRoot();
void handleColor();
void handleFade();
void handleStatus();

// --- 색상을 설정하는 함수 (공통 음극 LED용: 0=꺼짐, 255=밝음) ---
void setColor(int r, int g, int b) {
  ledcWrite(redChannel, r);
  ledcWrite(greenChannel, g);
  ledcWrite(blueChannel, b);
  currentRed = r;
  currentGreen = g;
  currentBlue = b;
}

// --- Courtesy http://www.instructables.com/id/How-to-Use-an-RGB-LED/?ALLSTEPS ---
// function to convert a color to its Red, Green, and Blue components.
void hueToRGB(uint8_t hue, uint8_t brightness) {
  uint16_t scaledHue = (hue * 6);
  uint8_t segment = scaledHue / 256;      // segment 0 to 5 around the
                                          // color wheel
  uint16_t segmentOffset = scaledHue - (segment * 256); // position within the segment

  uint8_t complement = 0;
  uint16_t prev = (brightness * (255 - segmentOffset)) / 256;
  uint16_t next = (brightness * segmentOffset) / 256;

  if (invert) { // KY-016은 Common Cathode이므로 이 블록은 실행되지 않음
    brightness = 255 - brightness;
    complement = 255;
    prev = 255 - prev;
    next = 255 - next;
  }

  switch (segment) {
    case 0:  // red
      R_val = brightness;
      G_val = next;
      B_val = complement;
      break;
    case 1:  // yellow
      R_val = prev;
      G_val = brightness;
      B_val = complement;
      break;
    case 2:  // green
      R_val = complement;
      G_val = brightness;
      B_val = next;
      break;
    case 3:  // cyan
      R_val = complement;
      G_val = prev;
      B_val = brightness;
      break;
    case 4:  // blue
      R_val = next;
      G_val = complement;
      B_val = brightness;
      break;
    case 5:  // magenta
    default:
      R_val = brightness;
      G_val = complement;
      B_val = prev;
      break;
  }
}

// --- 웹 페이지를 클라이언트에게 전송하는 함수 ---
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>ESP32 RGB LED Control</title>
      <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #282c34; color: #f0f0f0; }
        h1 { color: #61dafb; }
        .button {
          display: inline-block;
          padding: 15px 30px;
          font-size: 24px;
          margin: 10px;
          cursor: pointer;
          border-radius: 8px;
          border: none;
          color: white;
          text-decoration: none;
          box-shadow: 0 4px 8px rgba(0,0,0,0.2);
          transition: background-color 0.3s ease;
        }
        .button:hover { opacity: 0.9; }
        .on { background-color: #4CAF50; } /* Green */
        .off { background-color: #f44336; } /* Red */
        .blue { background-color: #008CBA; } /* Blue */
        .green { background-color: #00cc00; } /* Bright Green */
        .purple { background-color: #800080; } /* Purple */
        .white { background-color: #e0e0e0; color: #333;} /* White */
        .yellow { background-color: #ffc107; color: #333;} /* Yellow */
        .cyan { background-color: #00bcd4; } /* Cyan */
        .info { font-size: 18px; margin-top: 20px; color: #a0a0a0; }
        #current_color_display {
            display: inline-block;
            width: 50px;
            height: 50px;
            border-radius: 50%;
            border: 2px solid #61dafb;
            vertical-align: middle;
            margin-left: 10px;
        }
      </style>
    </head>
    <body>
      <h1>ESP32 RGB LED 제어</h1>
      <p class="info">현재 IP: )"rawliteral" + WiFi.localIP().toString() + R"rawliteral(</p>
      <p class="info">현재 LED 색상: <span id="current_color_text">RGB(0, 0, 0)</span> <span id="current_color_display"></span></p>
      
      <a href="/color?r=255&g=0&b=0" class="button on">빨강색</a>
      <a href="/color?r=0&g=255&b=0" class="button green">초록색</a>
      <a href="/color?r=0&g=0&b=255" class="button blue">파랑색</a>
      <br>
      <a href="/color?r=255&g=255&b=0" class="button yellow">노랑색</a>
      <a href="/color?r=0&g=255&b=255" class="button cyan">시안색</a>
      <a href="/color?r=255&g=0&b=255" class="button purple">보라색</a>
      <a href="/color?r=255&g=255&b=255" class="button white">흰색</a>
      <br>
      <a href="/fade" class="button">색상 페이드 시작</a>
      <a href="/color?r=0&g=0&b=0" class="button off">LED 끄기</a>

      <script>
        // 현재 색상 정보를 업데이트하는 함수
        function updateColorDisplay() {
          fetch('/status') // ESP32에서 현재 색상 상태를 가져옴
            .then(response => response.json())
            .then(data => {
              document.getElementById('current_color_text').innerText = `RGB(${data.r}, ${data.g}, ${data.b})`;
              document.getElementById('current_color_display').style.backgroundColor = `rgb(${data.r}, ${data.g}, ${data.b})`;
            })
            .catch(error => console.error('Error fetching status:', error));
        }

        // 페이지 로드 시, 그리고 2초마다 색상 정보 업데이트
        window.onload = updateColorDisplay;
        setInterval(updateColorDisplay, 2000);
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// 특정 색상 설정
void handleColor() {
  int r = server.arg("r").toInt(); // URL 파라미터에서 'r' 값 가져오기
  int g = server.arg("g").toInt(); // URL 파라미터에서 'g' 값 가져오기
  int b = server.arg("b").toInt(); // URL 파라미터에서 'b' 값 가져오기
  
  // 값 유효성 검사 (0~255 범위)
  r = constrain(r, 0, 255);
  g = constrain(g, 0, 255);
  b = constrain(b, 0, 255);

  setColor(r, g, b);
  String message = "Setting color: R(" + String(r) + ") G(" + String(g) + ") B(" + String(b) + ")";
  Serial.println(message);
  server.sendHeader("Location", "/"); // 메인 페이지로 리디렉션
  server.send(303); // 303 See Other
}

// 색상 페이드 루프 시작
void handleFade() {
  Serial.println("Starting color fade loop from web request.");
  server.sendHeader("Location", "/"); // 메인 페이지로 리디렉션
  server.send(303); // 303 See Other

  // 웹 요청으로 페이드를 시작하면, loop()에서 자동으로 실행되도록 플래그를 설정할 수도 있지만,
  // 여기서는 간단히 한 번의 페이드 사이클을 실행합니다.
  // 실제 사용에서는 loop()에서 주기적으로 실행되거나, 특정 이벤트에 반응하도록 하는 것이 좋습니다.
  for (colorHue = 0; colorHue < 255; colorHue++) {
    hueToRGB(colorHue, brightness_val);
    setColor(R_val, G_val, B_val);
    delay(50); // 페이드 속도 조절
  }
  setColor(0,0,0); // 페이드 끝나면 꺼짐
}


// 현재 LED 색상 상태를 JSON으로 반환
void handleStatus() {
  String json = "{\"r\":" + String(currentRed) + ",\"g\":" + String(currentGreen) + ",\"b\":" + String(currentBlue) + "}";
  server.send(200, "application/json", json);
}


void setup() {
  Serial.begin(115200);

  // --- LED PWM 채널 설정 (여기서 ledcSetup과 ledcAttachPin을 사용) ---
  ledcAttach(ledR, redChannel); // ledR (GPIO 18)을 redChannel에 연결

  ledcSetup(greenChannel, freq, resolution);
  ledcAttach(ledG, greenChannel); // ledG (GPIO 19)를 greenChannel에 연결

  ledcSetup(blueChannel, freq, resolution);
  ledcAttach(ledB, blueChannel); // ledB (GPIO 21)를 blueChannel에 연결

  // 초기 LED 상태 (꺼짐)
  setColor(0, 0, 0);

  // --- Wi-Fi 연결 ---
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wi-Fi 연결 대기
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // --- 웹 서버 라우팅 설정 ---
  server.on("/", handleRoot);        // 루트 경로 (메인 페이지)
  server.on("/color", handleColor);  // /color 경로 (색상 설정)
  server.on("/fade", handleFade);    // /fade 경로 (색상 페이드 시작)
  server.on("/status", handleStatus); // /status 경로 (현재 색상 상태 JSON 반환)

  server.begin(); // 웹 서버 시작
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // 웹 클라이언트 요청 처리
  // 다른 주기적인 작업을 여기에 추가할 수 있습니다.
  // 예를 들어, 특정 조건에서 자동으로 색상 페이드를 시작하는 등
}
