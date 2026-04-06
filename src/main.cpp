#include <Arduino.h>
#include "config.h"
#include "motors/MotorController.h"
#include "sensors/BatteryMonitor.h"
#include "sensors/DistanceSensor.h"
#include "safety/SafetyMonitor.h"
#include "leds/LEDController.h"
#include "bluetooth/BLEServer.h"
#include <ArduinoJson.h>

MotorController  motors;
BatteryMonitor   battery;
DistanceSensor   frontSensor(DIST_TRIG_FRONT, DIST_ECHO_FRONT, "FRONTAL");
DistanceSensor   rearSensor (DIST_TRIG_REAR,  DIST_ECHO_REAR,  "TRASERO");
SafetyMonitor    safety(frontSensor, rearSensor, motors);
LEDController    led;

Direction currentDir = Direction::STOP;
bool      brainWasOnline = false;
const char* currentAction = "idle";

enum class ActionStepKind { NONE, MOVE, STOP, LED };

struct ActionStep {
  ActionStepKind kind = ActionStepKind::NONE;
  Direction dir = Direction::STOP;
  uint32_t durationMs = 0;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  const char* actionName = "idle";
};

constexpr size_t MAX_SEQUENCE_STEPS = 16;

ActionStep activeStep;
ActionStep sequenceSteps[MAX_SEQUENCE_STEPS];
size_t sequenceStepCount = 0;
size_t sequenceStepIndex = 0;
unsigned long activeStepStartedAt = 0;
bool activeStepRunning = false;
bool sequenceRunning = false;

uint32_t clampCalculatedDuration(uint32_t durationMs) {
  if (durationMs == 0) {
    return 1;
  }

  return durationMs;
}

uint32_t durationFromCentimeters(float centimeters) {
  if (centimeters <= 0.0f) {
    return 0;
  }

  const float durationMs =
    (centimeters * static_cast<float>(MOTION_MS_PER_10_CM_AT_SPEED_200)) / 10.0f;
  return clampCalculatedDuration(static_cast<uint32_t>(durationMs + 0.5f));
}

uint32_t durationFromDegrees(float degrees) {
  if (degrees <= 0.0f) {
    return 0;
  }

  const float durationMs =
    (degrees * static_cast<float>(MOTION_MS_PER_90_DEG_AT_SPEED_200)) / 90.0f;
  return clampCalculatedDuration(static_cast<uint32_t>(durationMs + 0.5f));
}

void logInvalidMovementCommand(const char* type, const char* field) {
  Serial.printf("[BLE] Comando %s ignorado: %s invalido\n", type, field);
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

void setRobotIdleState() {
  motors.stop();
  currentDir = Direction::STOP;
  currentAction = "idle";
  led.setMode(LEDMode::IDLE);
}

bool buildActionStep(JsonObject& action, ActionStep& step) {
  const char* type = action["type"] | "";
  const uint32_t durationMs = action["duration_ms"] | 0;

  if (strcmp(type, "turn_right_deg") == 0) {
    const float degrees = action["degrees"] | 0.0f;
    const uint32_t calculatedDurationMs = durationFromDegrees(degrees);
    if (calculatedDurationMs == 0) {
      logInvalidMovementCommand(type, "degrees");
      return false;
    }

    step.kind = ActionStepKind::MOVE;
    step.dir = Direction::RIGHT;
    step.durationMs = calculatedDurationMs;
    step.actionName = "turn_right_deg";
    return true;
  }

  if (strcmp(type, "turn_left_deg") == 0) {
    const float degrees = action["degrees"] | 0.0f;
    const uint32_t calculatedDurationMs = durationFromDegrees(degrees);
    if (calculatedDurationMs == 0) {
      logInvalidMovementCommand(type, "degrees");
      return false;
    }

    step.kind = ActionStepKind::MOVE;
    step.dir = Direction::LEFT;
    step.durationMs = calculatedDurationMs;
    step.actionName = "turn_left_deg";
    return true;
  }

  if (strcmp(type, "move_forward_duration") == 0) {
    step.kind = ActionStepKind::MOVE;
    step.dir = Direction::FORWARD;
    step.durationMs = durationMs;
    step.actionName = "move_forward_duration";
    return true;
  }

  if (strcmp(type, "move_backward_duration") == 0) {
    step.kind = ActionStepKind::MOVE;
    step.dir = Direction::BACKWARD;
    step.durationMs = durationMs;
    step.actionName = "move_backward_duration";
    return true;
  }

  if (strcmp(type, "move_forward_cm") == 0) {
    const float centimeters = action["cm"] | 0.0f;
    const uint32_t calculatedDurationMs = durationFromCentimeters(centimeters);
    if (calculatedDurationMs == 0) {
      logInvalidMovementCommand(type, "cm");
      return false;
    }

    step.kind = ActionStepKind::MOVE;
    step.dir = Direction::FORWARD;
    step.durationMs = calculatedDurationMs;
    step.actionName = "move_forward_cm";
    return true;
  }

  if (strcmp(type, "move_backward_cm") == 0) {
    const float centimeters = action["cm"] | 0.0f;
    const uint32_t calculatedDurationMs = durationFromCentimeters(centimeters);
    if (calculatedDurationMs == 0) {
      logInvalidMovementCommand(type, "cm");
      return false;
    }

    step.kind = ActionStepKind::MOVE;
    step.dir = Direction::BACKWARD;
    step.durationMs = calculatedDurationMs;
    step.actionName = "move_backward_cm";
    return true;
  }

  if (strcmp(type, "stop") == 0) {
    step.kind = ActionStepKind::STOP;
    step.durationMs = durationMs;
    step.actionName = "stop";
    return true;
  }

  if (strcmp(type, "led_color") == 0) {
    step.kind = ActionStepKind::LED;
    step.durationMs = durationMs;
    step.r = action["r"] | 0;
    step.g = action["g"] | 0;
    step.b = action["b"] | 0;
    step.actionName = "led_color";
    return true;
  }

  return false;
}

void clearSequence() {
  sequenceRunning = false;
  sequenceStepCount = 0;
  sequenceStepIndex = 0;
}

void cancelActiveActions(bool setIdleLed = true) {
  activeStepRunning = false;
  activeStep = ActionStep{};
  clearSequence();
  motors.stop();
  currentDir = Direction::STOP;
  currentAction = "idle";
  if (setIdleLed) {
    led.setMode(LEDMode::IDLE);
  }
}

void startActionStep(const ActionStep& step) {
  activeStep = step;
  activeStepRunning = true;
  activeStepStartedAt = millis();
  currentAction = step.actionName;

  if (step.kind == ActionStepKind::MOVE) {
    currentDir = step.dir;
    led.setMode(LEDMode::MOVING);
    motors.move(step.dir, MOTION_FIXED_SPEED);
    return;
  }

  if (step.kind == ActionStepKind::STOP) {
    motors.stop();
    currentDir = Direction::STOP;
    led.setMode(LEDMode::IDLE);
    return;
  }

  currentDir = Direction::STOP;
  led.setCustom(step.r, step.g, step.b);
}

void advanceActionQueue() {
  if (sequenceRunning && sequenceStepIndex < sequenceStepCount) {
    startActionStep(sequenceSteps[sequenceStepIndex++]);
    return;
  }

  activeStepRunning = false;
  activeStep = ActionStep{};
  clearSequence();
  setRobotIdleState();
}

void completeActiveStep() {
  if (activeStep.kind == ActionStepKind::MOVE || activeStep.kind == ActionStepKind::STOP) {
    motors.stop();
  }

  if (!sequenceRunning || sequenceStepIndex >= sequenceStepCount) {
    setRobotIdleState();
  }

  advanceActionQueue();
}

void scheduleSingleAction(const ActionStep& step) {
  cancelActiveActions();
  startActionStep(step);
}

void scheduleSequence(JsonArray& steps) {
  cancelActiveActions();

  for (JsonObject stepJson : steps) {
    if (sequenceStepCount >= MAX_SEQUENCE_STEPS) {
      Serial.printf("[BLE] Secuencia truncada a %u pasos\n", MAX_SEQUENCE_STEPS);
      break;
    }

    ActionStep step;
    if (buildActionStep(stepJson, step)) {
      sequenceSteps[sequenceStepCount++] = step;
    } else {
      const char* type = stepJson["type"] | "";
      Serial.printf("[BLE] Paso de secuencia ignorado: %s\n", type);
    }
  }

  if (sequenceStepCount == 0) {
    return;
  }

  sequenceRunning = true;
  sequenceStepIndex = 0;
  advanceActionQueue();
}

void updateActiveAction() {
  if (!activeStepRunning) {
    return;
  }

  if (millis() - activeStepStartedAt < activeStep.durationMs) {
    return;
  }

  completeActiveStep();
}

void executePrimitiveAction(JsonObject& action) {
  ActionStep step;
  if (buildActionStep(action, step)) {
    scheduleSingleAction(step);
  }
}

String buildTelemetry() {
  StaticJsonDocument<768> doc;
  doc["type"]      = "telemetry";
  doc["timestamp"] = millis();

  BatteryReading battReading = battery.read();

  JsonObject batt = doc.createNestedObject("battery");
  batt["pack_voltage"] = battReading.packVoltage;
  batt["adc_voltage"]  = battReading.adcVoltage;
  batt["adc_mv"]       = battReading.adcMillivolts;
  batt["percentage"]   = battReading.percentage;
  batt["sensor_ok"]    = battReading.sensorOk;
  batt["measurement"]  = "voltage_divider";

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
  battery.begin();
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
      cancelActiveActions();
    },
    // onSequence
    [](JsonArray& steps) {
      scheduleSequence(steps);
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
    cancelActiveActions(false);
    led.setMode(LEDMode::BRAIN_OFFLINE);
  }
  if (brainOnline && !brainWasOnline) {
    Serial.println("[BLE] Brain recuperado");
    led.setMode(LEDMode::IDLE);
  }
  brainWasOnline = brainOnline;

  // Seguridad sensores de distancia
  static unsigned long lastSafety = 0;
  if (millis() - lastSafety > SAFETY_CHECK_INTERVAL_MS) {
    if (safety.check(currentDir)) {
      cancelActiveActions(false);
      led.setMode(LEDMode::ERROR_STATE);
    }
    lastSafety = millis();
  }

  updateActiveAction();

  // Telemetría periódica
  static unsigned long lastTelemetry = 0;
  if (millis() - lastTelemetry > TELEMETRY_INTERVAL_MS) {
    if (bleServer.isConnected()) {
      bleServer.sendTelemetry(buildTelemetry());
    }
    lastTelemetry = millis();
  }

  // Batería baja
  // if (battery.isLow()) led.setMode(LEDMode::LOW_BATTERY);
}