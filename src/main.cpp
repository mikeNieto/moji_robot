#include <Arduino.h>

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("tick");
    delay(1000);

    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("tock");
    delay(1000);
}