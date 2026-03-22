#pragma once
#include <Arduino.h>
#include "config.h"

enum class Direction { FORWARD, BACKWARD, LEFT, RIGHT, STOP };

class MotorController {
public:
  void begin();
  void move(Direction dir, uint8_t speed);
  void stop();
  void runFor(Direction dir, uint8_t speed, uint32_t durationMs);

private:
  uint8_t _currentSpeed = 0;
  void applyMotors(bool in1, bool in2, bool in3, bool in4, uint8_t speed);
  uint8_t rampStep(uint8_t target);
};