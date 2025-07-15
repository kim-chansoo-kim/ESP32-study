void setup() {
  // LED_BUILTIN 대신 직접 핀 번호 2를 사용
  pinMode(2, OUTPUT); 
}

void loop() {
  digitalWrite(2, HIGH);   // turn the LED on
  delay(1000);             // wait for a second
  digitalWrite(2, LOW);    // turn the LED off
  delay(1000);             // wait for a second
}