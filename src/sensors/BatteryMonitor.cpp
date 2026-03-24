#include "sensors/BatteryMonitor.h"

bool BatteryMonitor::begin() {
  Wire.begin(I2C_SDA, I2C_SCL);

  _connected = _ina219.begin();
  if (!_connected) {
    Serial.println("[BATTERY] ERROR: INA219 no detectado en 0x40");
    return false;
  }

  _ina219.setCalibration_32V_2A();
  Serial.println("[BATTERY] INA219 iniciado @ 0x40");
  return true;
}

bool BatteryMonitor::isConnected() const {
  return _connected;
}

uint8_t BatteryMonitor::percentageFromVoltage(float voltage) const {
  if (voltage >= BATTERY_VOLTAGE_MAX) return 100;
  if (voltage <= BATTERY_VOLTAGE_MIN) return 0;

  return (uint8_t)(100.0f * (voltage - BATTERY_VOLTAGE_MIN) /
                             (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN));
}

BatteryReading BatteryMonitor::read() {
  BatteryReading reading;
  reading.sensorOk = _connected;
  if (!_connected) return reading;

  reading.busVoltage   = _ina219.getBusVoltage_V();
  reading.shuntVoltage = _ina219.getShuntVoltage_mV();
  reading.loadVoltage  = reading.busVoltage + (reading.shuntVoltage / 1000.0f);
  reading.currentmA    = _ina219.getCurrent_mA();
  reading.powermW      = _ina219.getPower_mW();
  reading.percentage   = percentageFromVoltage(reading.loadVoltage);
  return reading;
}

float BatteryMonitor::readVoltage() {
  return read().loadVoltage;
}

float BatteryMonitor::readCurrentmA() {
  return read().currentmA;
}

float BatteryMonitor::readPowermW() {
  return read().powermW;
}

uint8_t BatteryMonitor::readPercentage() {
  return read().percentage;
}

bool BatteryMonitor::isLow() {
  if (!_connected) return false;
  return readPercentage() < LOW_BATTERY_THRESHOLD;
}