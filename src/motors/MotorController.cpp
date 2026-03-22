#include "motors/MotorController.h"

void MotorController::begin() {
  // Pines de dirección
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);

  // PWM — API ledcSetup/ledcAttachPin (ESP32 Arduino 2.x)
  ledcSetup(MOTOR_PWM_CHANNEL_A, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
  ledcAttachPin(MOTOR_ENA, MOTOR_PWM_CHANNEL_A);
  ledcSetup(MOTOR_PWM_CHANNEL_B, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
  ledcAttachPin(MOTOR_ENB, MOTOR_PWM_CHANNEL_B);

  stop();
  Serial.println("[MOTOR] MotorController iniciado");
}

void MotorController::applyMotors(bool in1, bool in2, bool in3, bool in4, uint8_t speed) {
  digitalWrite(MOTOR_IN1, in1);
  digitalWrite(MOTOR_IN2, in2);
  digitalWrite(MOTOR_IN3, in3);
  digitalWrite(MOTOR_IN4, in4);
  ledcWrite(MOTOR_PWM_CHANNEL_A, speed);
  ledcWrite(MOTOR_PWM_CHANNEL_B, speed);
  _currentSpeed = speed;
}

void MotorController::move(Direction dir, uint8_t speed) {
  switch (dir) {
    case Direction::FORWARD:  applyMotors(1,0,1,0, speed); break;
    case Direction::BACKWARD: applyMotors(0,1,0,1, speed); break;
    case Direction::LEFT:     applyMotors(0,1,1,0, speed); break;
    case Direction::RIGHT:    applyMotors(1,0,0,1, speed); break;
    case Direction::STOP:     stop(); break;
  }
}

void MotorController::stop() {
  applyMotors(0,0,0,0, 0);
  Serial.println("[MOTOR] STOP");
}

void MotorController::runFor(Direction dir, uint8_t speed, uint32_t durationMs) {
  move(dir, speed);
  delay(durationMs);
  stop();
}