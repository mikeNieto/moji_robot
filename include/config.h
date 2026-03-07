#pragma once

// ─── MOTORES ────────────────────────────────────────────────────────────────
#define MOTOR_IN1 41 // Motor Izquierdo → Adelante
#define MOTOR_IN2 42 // Motor Izquierdo → Atrás
#define MOTOR_IN3 47 // Motor Derecho  → Adelante
#define MOTOR_IN4 48 // Motor Derecho  → Atrás
#define MOTOR_ENA 1  // Enable Motor Izquierdo (PWM)
#define MOTOR_ENB 2  // Enable Motor Derecho   (PWM)

#define MOTOR_PWM_FREQ 1000    // Hz
#define MOTOR_PWM_RESOLUTION 8 // bits (0–255)
#define MOTOR_RAMP_STEP 10     // % por ciclo de rampa
#define MOTOR_RAMP_DELAY_MS 50 // ms entre pasos de rampa

// ─── SENSORES DISTANCIA HC-SR04 ─────────────────────────────────────────────
#define DIST_TRIG_FRONT 4
#define DIST_ECHO_FRONT 5
#define DIST_TRIG_REAR 6
#define DIST_ECHO_REAR 7

#define DISTANCE_THRESHOLD_CM 10    // Detener si obstáculo < 10 cm
#define ULTRASONIC_TIMEOUT_US 30000 // Timeout por lectura

// ─── LED RGB (ánodo común) ───────────────────────────────────────────────────
#define LED_PIN_R 38
#define LED_PIN_G 39
#define LED_PIN_B 40

#define LED_PWM_FREQ 1000 // Hz
#define LED_PWM_CHANNEL_R 0
#define LED_PWM_CHANNEL_G 1
#define LED_PWM_CHANNEL_B 2
#define LED_PWM_RESOLUTION 8 // 8 bits → 0–255

// ─── BLUETOOTH ──────────────────────────────────────────────────────────────
#define BLE_DEVICE_NAME "RobotESP32"
#define BLE_MTU_SIZE 512
#define HEARTBEAT_TIMEOUT_MS 3000  // 3s sin heartbeat → BRAIN_OFFLINE
#define HEARTBEAT_INTERVAL_MS 1000 // Se espera un heartbeat cada 1s

// ─── SENSORES CLIFF VL53L0X (I²C) ───────────────────────────────────────────
#define I2C_SDA 21
#define I2C_SCL 22
#define XSHUT_CLIFF_FL 11 // Front-Left
#define XSHUT_CLIFF_FR 12 // Front-Right
#define XSHUT_CLIFF_RR 13 // Rear

#define CLIFF_THRESHOLD_MM 80 // < 80mm al suelo = vacío (precipicio)
#define CLIFF_CHECK_INTERVAL_MS 100

// ─── BATERÍA ─────────────────────────────────────────────────────────────────
#define BATTERY_ADC_PIN 8             // ADC1_CH7, divisor 82kΩ / 47kΩ
#define BATTERY_ADC_SAMPLES 10        // Promedio de lecturas
#define BATTERY_VOLTAGE_MIN 6.0f      // V (3.0V × 2S)
#define BATTERY_VOLTAGE_MAX 8.4f      // V (4.2V × 2S)
#define BATTERY_DIVIDER_RATIO 2.7447f // (82k+47k)/47k
#define LOW_BATTERY_THRESHOLD 10      // %

// ─── TELEMETRÍA ──────────────────────────────────────────────────────────────
#define TELEMETRY_INTERVAL_MS 1000 // Enviar telemetría cada 1s
