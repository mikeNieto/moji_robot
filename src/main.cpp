#include <Arduino.h>
#include "config.h"
#include "motors/MotorController.h"
// #include "sensors/BatteryMonitor.h"
#include "sensors/DistanceSensor.h"
#include "safety/SafetyMonitor.h"
#include "leds/LEDController.h"
#include "bluetooth/BLEServer.h"
#include <ArduinoJson.h>

MotorController  motors;
// BatteryMonitor   battery;
DistanceSensor   frontSensor(DIST_TRIG_FRONT, DIST_ECHO_FRONT, "FRONTAL");
DistanceSensor   rearSensor (DIST_TRIG_REAR,  DIST_ECHO_REAR,  "TRASERO");
SafetyMonitor    safety(frontSensor, rearSensor, motors);
LEDController    led;

Direction currentDir = Direction::STOP;
bool      brainWasOnline = false;
const char* currentAction = "idle";

uint8_t normalizeMotionSpeed(int requestedSpeed) {
  if (requestedSpeed > 255) {
    return 255;
  }

  if (requestedSpeed > 0 && requestedSpeed < 130) {
    return 130;
  }

  if (requestedSpeed < 0) {
    return 130;
  }

  return static_cast<uint8_t>(requestedSpeed);
}

const char* directionToString(Direction dir) {
  switch (dir) {
    case Direction::FORWARD:  return "forward";
    case Direction::BACKWARD: return "backward";
    case Direction::LEFT:     return "left";
    case Direction::RIGHT:    return "right";
    default:                  return "stop";
  }
}

void finishMotionAction() {
  motors.stop();
  currentDir = Direction::STOP;
    currentAction = "idle";
    led.setMode(LEDMode::IDLE);
}

void executePrimitiveAction(JsonObject& action) {
  const char* type       = action["type"] | "";
  int         requestedSpeed = action["speed"] | 0;
  uint32_t    durationMs = action["duration_ms"] | 0;

  if (strcmp(type, "turn_right_deg") == 0) {
    uint8_t speed = normalizeMotionSpeed(requestedSpeed);
    currentAction = "turn_right_deg";
    currentDir = Direction::RIGHT;
    led.setMode(LEDMode::MOVING);
    motors.runFor(Direction::RIGHT, speed, durationMs);
    finishMotionAction();

  } else if (strcmp(type, "turn_left_deg") == 0) {
    uint8_t speed = normalizeMotionSpeed(requestedSpeed);
    currentAction = "turn_left_deg";
    currentDir = Direction::LEFT;
    led.setMode(LEDMode::MOVING);
    motors.runFor(Direction::LEFT, speed, durationMs);
    finishMotionAction();

  } else if (strcmp(type, "move_forward_cm") == 0) {
    uint8_t speed = normalizeMotionSpeed(requestedSpeed);
    currentAction = "move_forward_cm";
    currentDir = Direction::FORWARD;
    led.setMode(LEDMode::MOVING);
    motors.runFor(Direction::FORWARD, speed, durationMs);
    finishMotionAction();

  } else if (strcmp(type, "move_backward_cm") == 0) {
    uint8_t speed = normalizeMotionSpeed(requestedSpeed);
    currentAction = "move_backward_cm";
    currentDir = Direction::BACKWARD;
    led.setMode(LEDMode::MOVING);
    motors.runFor(Direction::BACKWARD, speed, durationMs);
    finishMotionAction();

  } else if (strcmp(type, "led_color") == 0) {
    currentAction = "led_color";
    led.setCustom(action["r"] | 0, action["g"] | 0, action["b"] | 0);
    if (durationMs > 0) {
      delay(durationMs);
      currentAction = "idle";
      led.setMode(LEDMode::IDLE);
    }
  }
}

String buildTelemetry() {
  StaticJsonDocument<768> doc;
  doc["type"]      = "telemetry";
  doc["timestamp"] = millis();

//   BatteryReading battReading = battery.read();

  JsonObject batt = doc.createNestedObject("battery");
//   batt["bus_voltage"]      = battReading.busVoltage;
//   batt["load_voltage"]     = battReading.loadVoltage;
//   batt["shunt_voltage_mv"] = battReading.shuntVoltage;
//   batt["current_ma"]       = battReading.currentmA;
//   batt["power_mw"]         = battReading.powermW;
//   batt["percentage"]       = battReading.percentage;
//   batt["sensor_ok"]        = battReading.sensorOk;

  JsonObject sens = doc.createNestedObject("sensors");
  sens["distance_front"] = frontSensor.readCm();
  sens["distance_rear"]  = rearSensor.readCm();

  JsonObject motorsJson = doc.createNestedObject("motors");
  motorsJson["state"] = directionToString(currentDir);
  motorsJson["last_action"] = currentAction;

  JsonObject heartbeat = doc.createNestedObject("heartbeat");
  heartbeat["brain_online"] = bleServer.isBrainOnline();

  JsonObject safetyJson = doc.createNestedObject("safety");
  safetyJson["cliff_active"] = false;
  safetyJson["obstacle_blocked"] =
    (currentDir == Direction::FORWARD && frontSensor.isObstacle()) ||
    (currentDir == Direction::BACKWARD && rearSensor.isObstacle());

  doc["uptime"] = millis() / 1000;

  String out;
  serializeJson(doc, out);
  return out;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== MOJI ESP32 — Etapa 4: BLE ===");

  motors.begin();
  // battery.begin();
  frontSensor.begin();
  rearSensor.begin();
  led.begin();
  led.setMode(LEDMode::IDLE);

  // Registrar callbacks BLE
  bleServer.registerCallbacks(
    // onPrimitive
    [](JsonObject& action) {
      executePrimitiveAction(action);
    },
    // onStop
    []() {
      currentAction = "stop";
      currentDir = Direction::STOP;
      motors.stop();
      led.setMode(LEDMode::IDLE);
    },
    // onSequence
    [](JsonArray& steps) {
      for (JsonObject step : steps) {
        executePrimitiveAction(step);
      }
      currentAction = "idle";
      finishMotionAction();
    },
    // onTelemetryRequest
    []() {
      if (bleServer.isConnected()) {
        bleServer.sendTelemetry(buildTelemetry());
      }
    }
  );

  bleServer.begin();
}

void loop() {
  led.update();
  bleServer.update();

  // Gestión BRAIN_OFFLINE
  bool brainOnline = bleServer.isBrainOnline();
  if (bleServer.isConnected() && !brainOnline && brainWasOnline) {
    Serial.println("[SAFETY] BRAIN_OFFLINE → STOP + LED ámbar");
    motors.stop();
    currentDir = Direction::STOP;
    led.setMode(LEDMode::BRAIN_OFFLINE);
  }
  if (brainOnline && !brainWasOnline) {
    Serial.println("[BLE] Brain recuperado");
    led.setMode(LEDMode::IDLE);
  }
  brainWasOnline = brainOnline;

  // Seguridad sensores de distancia
  static unsigned long lastSafety = 0;
  if (millis() - lastSafety > 100) {
    if (safety.check(currentDir)) {
      currentDir = Direction::STOP;
      led.setMode(LEDMode::ERROR_STATE);
    }
    lastSafety = millis();
  }

  // Telemetría periódica
  static unsigned long lastTelemetry = 0;
  if (millis() - lastTelemetry > TELEMETRY_INTERVAL_MS) {
    if (bleServer.isConnected()) {
      bleServer.sendTelemetry(buildTelemetry());
    }
    lastTelemetry = millis();
  }

  // Batería baja
//   if (battery.isLow()) led.setMode(LEDMode::LOW_BATTERY);
}