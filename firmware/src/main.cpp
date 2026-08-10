#include <Arduino.h>

const int LED_PIN = 2;

void setup(){
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Telemetry logger: boot OK");
}

void loop(){
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    Serial.println("blink");
}