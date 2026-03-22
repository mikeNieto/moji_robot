#pragma once
#include <Arduino.h>
#include "config.h"

enum class LEDMode { IDLE, MOVING, ERROR_STATE, BRAIN_OFFLINE, LOW_BATTERY, CUSTOM };

class LEDController {
public:
  void begin();
  void setMode(LEDMode mode);
  void setCustom(uint8_t r, uint8_t g, uint8_t b);
  // Llamar periódicamente para actualizar efectos animados
  void update();

private:
  LEDMode  _mode    = LEDMode::IDLE;
  uint8_t  _r = 0, _g = 0, _b = 0;
  uint32_t _lastUpdate = 0;
  float    _breathePhase = 0.0f;
  bool     _blinkState   = false;

  void writeRGB(uint8_t r, uint8_t g, uint8_t b);
  void animateBreathe(uint8_t r, uint8_t g, uint8_t b, float speed);
  void animateBlink(uint8_t r, uint8_t g, uint8_t b, uint32_t periodMs);
};