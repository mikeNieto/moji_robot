#include "sensors/BatteryMonitor.h"

bool BatteryMonitor::begin() {
  pinMode(BATTERY_SENSE_PIN, INPUT);
  analogReadResolution(BATTERY_ADC_RESOLUTION_BITS);
  analogSetPinAttenuation(BATTERY_SENSE_PIN, ADC_11db);

  _connected = true;
  Serial.printf("[BATTERY] Divisor resistivo iniciado en GPIO %d (ratio=%.4f)\n",
                BATTERY_SENSE_PIN,
                BATTERY_DIVIDER_RATIO);
  return _connected;
}

bool BatteryMonitor::isConnected() const {
  return _connected;
}

float BatteryMonitor::calculatePackVoltage(uint16_t adcMillivolts) const {
  const float adcVoltage = static_cast<float>(adcMillivolts) / 1000.0f;
  return (adcVoltage / BATTERY_DIVIDER_RATIO) * BATTERY_ADC_CALIBRATION_FACTOR;
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

  uint32_t totalMillivolts = 0;
  for (uint8_t sample = 0; sample < BATTERY_SENSE_SAMPLES; ++sample) {
    totalMillivolts += analogReadMilliVolts(BATTERY_SENSE_PIN);
    delayMicroseconds(200);
  }

  reading.adcMillivolts = static_cast<uint16_t>(totalMillivolts / BATTERY_SENSE_SAMPLES);
  reading.adcVoltage = static_cast<float>(reading.adcMillivolts) / 1000.0f;
  reading.packVoltage = calculatePackVoltage(reading.adcMillivolts);
  reading.percentage = percentageFromVoltage(reading.packVoltage);
  return reading;
}

float BatteryMonitor::readVoltage() {
  return read().packVoltage;
}

uint8_t BatteryMonitor::readPercentage() {
  return read().percentage;
}

bool BatteryMonitor::isLow() {
  if (!_connected) return false;
  return readPercentage() < LOW_BATTERY_THRESHOLD;
}