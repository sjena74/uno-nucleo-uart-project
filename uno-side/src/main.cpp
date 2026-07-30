#include <Arduino.h>
#include <HCSR04.h>

HCSR04 hc(2, 3);
const int PIR_PIN = 4;
const int PHOTO_PIN = A0;
void setup() {
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);
}

void loop() {
  Serial.print("Distance: ");
  Serial.print(hc.dist()); // method returns distance in cm
  Serial.println(" cm");

  int motion = digitalRead(PIR_PIN); // returns HIGH/LOW
  Serial.print("Motion: ");
  Serial.println(motion == HIGH ? "detected" : "not detected");

  int light = analogRead(PHOTO_PIN);
  Serial.print("Light: ");
  Serial.println(light);

  delay(500);
}

