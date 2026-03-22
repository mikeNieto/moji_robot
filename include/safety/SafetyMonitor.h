#pragma once
#include <Arduino.h>
#include "sensors/DistanceSensor.h"
#include "motors/MotorController.h"
#include "config.h"

class SafetyMonitor {
public:
  SafetyMonitor(DistanceSensor& front, DistanceSensor& rear, MotorController& motors);
  // Llamar periódicamente desde el loop. Devuelve true si hay emergencia.
  bool check(Direction currentDir);

private:
  DistanceSensor& _front;
  DistanceSensor& _rear;
  MotorController& _motors;
  Direction _lastDir = Direction::STOP;
  uint8_t _frontObstacleReads = 0;
  uint8_t _rearObstacleReads = 0;
};