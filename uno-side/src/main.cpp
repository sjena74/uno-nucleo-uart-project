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

void setup() {
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);

  lcd.begin(16, 2);
  lcd.print("Hello, Sohan!");
}

void loop() {
  Serial.print("Distance: ");
  Serial.print(hc.dist());
  Serial.println(" cm");

  int motion = digitalRead(PIR_PIN);
  Serial.print("Motion: ");
  Serial.println(motion == HIGH ? "detected" : "not detected");

  int light = analogRead(PHOTO_PIN);
  Serial.print("Light: ");
  Serial.println(light);

  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);

  digitalWrite(RELAY_PIN, HIGH);
  delay(1000);
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);

  myServo.write(0);
  delay(1000);
  myServo.write(90);
  delay(1000);
}