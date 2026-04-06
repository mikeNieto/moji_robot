#pragma once
#include <Arduino.h>
#include "config.h"

struct BatteryReading {
  float adcVoltage = 0.0f;
  float packVoltage = 0.0f;
  uint16_t adcMillivolts = 0;
  uint8_t percentage = 0;
  bool sensorOk = false;
};

class BatteryMonitor {
public:
  bool begin();
  bool isConnected() const;
  BatteryReading read();
  float readVoltage();
  uint8_t readPercentage();
  bool isLow();

private:
  bool _connected = false;

  float calculatePackVoltage(uint16_t adcMillivolts) const;
  uint8_t percentageFromVoltage(float voltage) const;
};