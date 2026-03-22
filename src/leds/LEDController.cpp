#include "leds/LEDController.h"
#include <math.h>

void LEDController::begin() {
  ledcSetup(LED_PWM_CHANNEL_R, LED_PWM_FREQ, LED_PWM_RESOLUTION);
  ledcAttachPin(LED_PIN_R, LED_PWM_CHANNEL_R);
  ledcSetup(LED_PWM_CHANNEL_G, LED_PWM_FREQ, LED_PWM_RESOLUTION);
  ledcAttachPin(LED_PIN_G, LED_PWM_CHANNEL_G);
  ledcSetup(LED_PWM_CHANNEL_B, LED_PWM_FREQ, LED_PWM_RESOLUTION);
  ledcAttachPin(LED_PIN_B, LED_PWM_CHANNEL_B);
  writeRGB(0, 0, 0);
  Serial.println("[LED] LEDController iniciado");
}

// ánodo común: invertir valores
void LEDController::writeRGB(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(LED_PWM_CHANNEL_R, 255 - r);
  ledcWrite(LED_PWM_CHANNEL_G, 255 - g);
  ledcWrite(LED_PWM_CHANNEL_B, 255 - b);
}

void LEDController::setMode(LEDMode mode) {
  _mode = mode;
  _breathePhase = 0.0f;
  _blinkState   = false;
}

void LEDController::setCustom(uint8_t r, uint8_t g, uint8_t b) {
  _mode = LEDMode::CUSTOM;
  _r = r; _g = g; _b = b;
}

void LEDController::animateBreathe(uint8_t r, uint8_t g, uint8_t b, float speed) {
  // Ciclo ~3s a speed=1.0, más rápido con speed mayor
  _breathePhase += speed * 0.02f;
  if (_breathePhase > TWO_PI) _breathePhase -= TWO_PI;
  float factor = (sin(_breathePhase) + 1.0f) / 2.0f;  // 0.0 – 1.0
  writeRGB((uint8_t)(r * factor), (uint8_t)(g * factor), (uint8_t)(b * factor));
}

void LEDController::animateBlink(uint8_t r, uint8_t g, uint8_t b, uint32_t periodMs) {
  if (millis() - _lastUpdate > periodMs / 2) {
    _blinkState = !_blinkState;
    _lastUpdate = millis();
  }
  writeRGB(_blinkState ? r : 0, _blinkState ? g : 0, _blinkState ? b : 0);
}

void LEDController::update() {
  switch (_mode) {
    case LEDMode::IDLE:
      // Azul suave, respiración lenta (~3s)
      animateBreathe(0, 80, 200, 0.8f);
      break;
    case LEDMode::MOVING:
      // Verde sólido
      writeRGB(0, 200, 0);
      break;
    case LEDMode::ERROR_STATE:
      // Rojo parpadeante rápido (2 Hz → 250ms por semiciclo)
      animateBlink(255, 0, 0, 500);
      break;
    case LEDMode::BRAIN_OFFLINE:
      // Ámbar pulsante suave (1 Hz)
      animateBreathe(255, 160, 0, 1.5f);
      break;
    case LEDMode::LOW_BATTERY:
      // Naranja parpadeante lento (0.5 Hz → 1000ms por semiciclo)
      animateBlink(255, 100, 0, 2000);
      break;
    case LEDMode::CUSTOM:
      writeRGB(_r, _g, _b);
      break;
  }
}