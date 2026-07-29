#include <Arduino.h>
#include <HCSR04.h>

HCSR04 hc(2, 3);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("Distance: ");
  Serial.print(hc.dist());
  Serial.println(" cm");
  delay(60);
}

