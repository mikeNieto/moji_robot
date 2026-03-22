#include "sensors/DistanceSensor.h"

DistanceSensor::DistanceSensor(uint8_t trigPin, uint8_t echoPin, const char* name)
  : _trig(trigPin), _echo(echoPin), _name(name) {}

void DistanceSensor::begin() {
  pinMode(_trig, OUTPUT);
  pinMode(_echo, INPUT);
  digitalWrite(_trig, LOW);
  Serial.printf("[DIST] %s iniciado (Trig=%d Echo=%d)\n", _name, _trig, _echo);
}

float DistanceSensor::readCm() {
  // Pulso de 10µs en Trig
  digitalWrite(_trig, LOW);  delayMicroseconds(2);
  digitalWrite(_trig, HIGH); delayMicroseconds(10);
  digitalWrite(_trig, LOW);

  long duration = pulseIn(_echo, HIGH, ULTRASONIC_TIMEOUT_US);
  if (duration == 0) return -1.0f;  // Timeout

  return duration * 0.0343f / 2.0f;  // cm (sonido a 343 m/s)
}

bool DistanceSensor::isObstacle() {
  float d = readCm();
  if (d < 0) return false;  // Timeout → asumir libre
  return d < DISTANCE_THRESHOLD_CM;
}