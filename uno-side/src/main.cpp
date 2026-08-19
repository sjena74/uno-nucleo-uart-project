#include <Arduino.h>
#include <HCSR04.h>
#include <Servo.h>
#include <LiquidCrystal.h>

HCSR04 hc(2, 3);
Servo myServo;
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

const int PIR_PIN = 4;
const int PHOTO_PIN = A0;
const int LED_PIN = 5;
const int RELAY_PIN = 6;
const int SERVO_PIN = 7;

const uint8_t START_BYTE = 0xAA;

// Sensor report types (UNO -> Nucleo)
const uint8_t TYPE_ULTRASONIC = 0x01;
const uint8_t TYPE_PIR        = 0x02;
const uint8_t TYPE_PHOTO      = 0x03;

// Command types (Nucleo -> UNO)
const uint8_t CMD_RELAY    = 0x10;
const uint8_t CMD_LED      = 0x11;
const uint8_t CMD_SERVO    = 0x12;
const uint8_t CMD_LCD_TEXT = 0x13;

uint8_t computeChecksum(uint8_t type, uint8_t len, uint8_t* data) {
  uint8_t sum = type ^ len;
  for (int i = 0; i < len; i++) sum ^= data[i];
  return sum;
}

void sendPacket(uint8_t type, uint8_t* data, uint8_t len) {
  Serial.write(START_BYTE);
  Serial.write(type);
  Serial.write(len);
  for (int i = 0; i < len; i++) Serial.write(data[i]);
  Serial.write(computeChecksum(type, len, data));
}

void sendUltrasonic() {
  uint16_t d = (uint16_t)hc.dist();
  uint8_t data[2] = { (uint8_t)(d >> 8), (uint8_t)(d & 0xFF) };
  sendPacket(TYPE_ULTRASONIC, data, 2);
}

void sendPIR() {
  uint8_t data[1] = { (uint8_t)digitalRead(PIR_PIN) };
  sendPacket(TYPE_PIR, data, 1);
}

void sendPhoto() {
  uint16_t v = analogRead(PHOTO_PIN);
  uint8_t data[2] = { (uint8_t)(v >> 8), (uint8_t)(v & 0xFF) };
  sendPacket(TYPE_PHOTO, data, 2);
}

void handleCommandPacket() {
  while (!Serial.available()) {}
  uint8_t type = Serial.read();

  while (!Serial.available()) {}
  uint8_t len = Serial.read();

  uint8_t data[16];
  for (int i = 0; i < len; i++) {
    while (!Serial.available()) {}
    data[i] = Serial.read();
  }

  while (!Serial.available()) {}
  uint8_t checksum = Serial.read();

  if (checksum != computeChecksum(type, len, data)) return; // drop corrupted packet

  switch (type) {
    case CMD_RELAY:
      digitalWrite(RELAY_PIN, data[0] ? HIGH : LOW);
      break;
    case CMD_LED:
      analogWrite(LED_PIN, data[0]);
      break;
    case CMD_SERVO:
      myServo.write(data[0]);
      break;
    case CMD_LCD_TEXT:
      lcd.clear();
      lcd.setCursor(0, 0);
      for (int i = 0; i < len; i++) lcd.write((char)data[i]);
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);
  lcd.begin(16, 2);
}

void loop() {
  if (Serial.available()) {
    uint8_t b = Serial.read();
    if (b == START_BYTE) handleCommandPacket();
    else if (b == TYPE_ULTRASONIC) sendUltrasonic();
    else if (b == TYPE_PIR) sendPIR();
    else if (b == TYPE_PHOTO) sendPhoto();
  }
}