#include <Arduino.h>
#include "config.h"
#include "motors/MotorController.h"
// #include "sensors/BatteryMonitor.h"
#include "sensors/DistanceSensor.h"
#include "safety/SafetyMonitor.h"
#include "leds/LEDController.h"

MotorController  motors;
// BatteryMonitor   battery;
DistanceSensor   frontSensor(DIST_TRIG_FRONT, DIST_ECHO_FRONT, "FRONTAL");
DistanceSensor   rearSensor (DIST_TRIG_REAR,  DIST_ECHO_REAR,  "TRASERO");
SafetyMonitor    safety(frontSensor, rearSensor, motors);
LEDController    led;

Direction currentDir = Direction::STOP;

void printMenu() {
  Serial.println("\n=== TEST MOTORES + SENSORES + LED ===");
  Serial.println("w/s/a/d → Mover | x → STOP");
  Serial.println("1 → IDLE | 2 → MOVING | 3 → ERROR | 4 → BRAIN_OFFLINE | 5 → LOW_BATT");
  Serial.println("r → Sensores | b → Batería");
  Serial.println("=====================================");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== MOJI ESP32 — Etapa 3: LED RGB ===");
  motors.begin();
//   battery.begin();
  frontSensor.begin();
  rearSensor.begin();
  led.begin();
  led.setMode(LEDMode::IDLE);
  printMenu();
}

void loop() {
  led.update();  // Actualizar efectos LED

  // Seguridad
  static unsigned long lastSafety = 0;
  if (millis() - lastSafety >= SAFETY_CHECK_INTERVAL_MS) {
    if (safety.check(currentDir)) {
      currentDir = Direction::STOP;
      led.setMode(LEDMode::ERROR_STATE);
    }
    lastSafety = millis();
  }

  // Batería baja
//   if (battery.isLow()) led.setMode(LEDMode::LOW_BATTERY);

  // Log periódico
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 2000) {
    Serial.printf("[STATUS] Frontal: %.1f cm | Trasero: %.1f cm\n",
                  frontSensor.readCm(), rearSensor.readCm());
    // Serial.printf("[STATUS] Frontal: %.1f cm | Trasero: %.1f cm | Batt: %d%%\n",
    //               frontSensor.readCm(), rearSensor.readCm(), battery.readPercentage());
    lastLog = millis();
  }

  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'w': currentDir = Direction::FORWARD;  motors.move(currentDir, 150); led.setMode(LEDMode::MOVING); break;
      case 's': currentDir = Direction::BACKWARD; motors.move(currentDir, 150); led.setMode(LEDMode::MOVING); break;
      case 'a': currentDir = Direction::LEFT;     motors.move(currentDir, 150); led.setMode(LEDMode::MOVING); break;
      case 'd': currentDir = Direction::RIGHT;    motors.move(currentDir, 150); led.setMode(LEDMode::MOVING); break;
      case 'x': currentDir = Direction::STOP;     motors.stop();                led.setMode(LEDMode::IDLE);   break;
      case '1': led.setMode(LEDMode::IDLE);          Serial.println("→ LED IDLE");          break;
      case '2': led.setMode(LEDMode::MOVING);        Serial.println("→ LED MOVING");        break;
      case '3': led.setMode(LEDMode::ERROR_STATE);   Serial.println("→ LED ERROR");         break;
      case '4': led.setMode(LEDMode::BRAIN_OFFLINE); Serial.println("→ LED BRAIN_OFFLINE"); break;
      case '5': led.setMode(LEDMode::LOW_BATTERY);   Serial.println("→ LED LOW_BATTERY");   break;
      case 'r':
        Serial.printf("[DIST] Frontal: %.1f cm | Trasero: %.1f cm\n",
                      frontSensor.readCm(), rearSensor.readCm()); break;
      case 'b':
        // Serial.printf("[BATT] %.2f V | %d%%\n",
        //               battery.readVoltage(), battery.readPercentage()); break;
      default: break;
    }
    printMenu();
  }
}