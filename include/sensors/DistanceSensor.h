#pragma once
#include <Arduino.h>
#include "config.h"

class DistanceSensor {
public:
  DistanceSensor(uint8_t trigPin, uint8_t echoPin, const char* name);
  void begin();
  float readCm();       // Distancia en centímetros (-1 si timeout)
  bool isObstacle();    // true si distancia < DISTANCE_THRESHOLD_CM

private:
  uint8_t  _trig, _echo;
  const char* _name;
};