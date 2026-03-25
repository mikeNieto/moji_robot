#include "safety/SafetyMonitor.h"

SafetyMonitor::SafetyMonitor(DistanceSensor& front, DistanceSensor& rear, MotorController& motors)
  : _front(front), _rear(rear), _motors(motors) {}

bool SafetyMonitor::check(Direction currentDir) {
  float frontDistance = -1.0f;
  float rearDistance = -1.0f;

  if (currentDir != _lastDir) {
    _frontObstacleReads = 0;
    _rearObstacleReads = 0;
    _lastDir = currentDir;
  }

  if (currentDir == Direction::FORWARD || currentDir == Direction::BACKWARD) {
    frontDistance = _front.readCm();
    rearDistance = _rear.readCm();
    Serial.printf("[DIST] front=%.1f cm rear=%.1f cm dir=%s\n",
                  frontDistance,
                  rearDistance,
                  currentDir == Direction::FORWARD ? "FORWARD" : "BACKWARD");
  }

  if (currentDir == Direction::FORWARD) {
    float distance = frontDistance;
    if (distance >= 0.0f && distance < DISTANCE_THRESHOLD_CM) {
      if (++_frontObstacleReads >= SAFETY_CONFIRM_READS) {
        Serial.printf("[SAFETY] Obstáculo FRONTAL %.1f cm -> STOP\n", distance);
        _motors.stop();
        _frontObstacleReads = 0;
        return true;
      }
    } else {
      _frontObstacleReads = 0;
    }
    _rearObstacleReads = 0;
  }

  if (currentDir == Direction::BACKWARD) {
    float distance = rearDistance;
    if (distance >= 0.0f && distance < DISTANCE_THRESHOLD_CM) {
      if (++_rearObstacleReads >= SAFETY_CONFIRM_READS) {
        Serial.printf("[SAFETY] Obstáculo TRASERO %.1f cm -> STOP\n", distance);
        _motors.stop();
        _rearObstacleReads = 0;
        return true;
      }
    } else {
      _rearObstacleReads = 0;
    }
    _frontObstacleReads = 0;
  }

  if (currentDir == Direction::STOP || currentDir == Direction::LEFT || currentDir == Direction::RIGHT) {
    _frontObstacleReads = 0;
    _rearObstacleReads = 0;
  }

  return false;
}