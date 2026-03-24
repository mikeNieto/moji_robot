#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "config.h"

struct BatteryReading {
  float busVoltage = 0.0f;
  float shuntVoltage = 0.0f;
  float loadVoltage = 0.0f;
  float currentmA = 0.0f;
  float powermW = 0.0f;
  uint8_t percentage = 0;
  bool sensorOk = false;
};

class BatteryMonitor {
public:
  bool begin();
  bool isConnected() const;
  BatteryReading read();
  float readVoltage();
  float readCurrentmA();
  float readPowermW();
  uint8_t readPercentage();
  bool isLow();

private:
  Adafruit_INA219 _ina219 = Adafruit_INA219(INA219_I2C_ADDRESS);
  bool _connected = false;

  uint8_t percentageFromVoltage(float voltage) const;
};