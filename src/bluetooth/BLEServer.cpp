#include "bluetooth/BLEServer.h"

#define SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_TX_UUID  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Write (Android → ESP32)
#define CHAR_RX_UUID  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Notify (ESP32 → Android)

RobotBLEServer bleServer;

void RobotBLEServer::begin() {
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEDevice::setMTU(BLE_MTU_SIZE);

  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(this);

  BLEService* pService = pServer->createService(SERVICE_UUID);

  // TX — recibe comandos desde Android
  BLECharacteristic* pTxChar = pService->createCharacteristic(
    CHAR_TX_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  pTxChar->setCallbacks(this);

  // RX — envía notificaciones a Android
  _pRxChar = pService->createCharacteristic(
    CHAR_RX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  _pRxChar->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising* pAdv = BLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->setScanResponse(true);
  pAdv->setMinPreferred(0x06);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising como 'RobotESP32'...");
}

void RobotBLEServer::registerCallbacks(PrimitiveCallback        onPrimitive,
                                       StopCallback             onStop,
                                       SequenceCallback         onSequence,
                                       TelemetryRequestCallback onTelemetryRequest) {
  _onPrimitive        = onPrimitive;
  _onStop             = onStop;
  _onSequence         = onSequence;
  _onTelemetryRequest = onTelemetryRequest;
}

void RobotBLEServer::onConnect(BLEServer* server) {
  _connected     = true;
  _lastHeartbeat = millis();
  Serial.println("[BLE] Cliente conectado");
}

void RobotBLEServer::onDisconnect(BLEServer* server) {
  _connected = false;
  Serial.println("[BLE] Cliente desconectado → restart advertising");
  BLEDevice::startAdvertising();
}

void RobotBLEServer::onWrite(BLECharacteristic* characteristic) {
  String value = characteristic->getValue().c_str();
  if (value.length() > 0) {
    handleCommand(value);
  }
}

void RobotBLEServer::handleCommand(const String& json) {
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[BLE] JSON error: %s\n", err.c_str());
    return;
  }

  const char* type = doc["type"];
  Serial.printf("[BLE] CMD recibido: %s\n", type);

  if (strcmp(type, "heartbeat") == 0) {
    _lastHeartbeat = millis();

  } else if (strcmp(type, "stop") == 0 && _onStop) {
    _onStop();

  } else if (strcmp(type, "move_sequence") == 0 && _onSequence) {
    JsonArray steps = doc["steps"];
    _onSequence(steps);

  } else if ((strcmp(type, "turn_right_deg") == 0 ||
              strcmp(type, "turn_left_deg") == 0 ||
              strcmp(type, "move_forward_cm") == 0 ||
              strcmp(type, "move_backward_cm") == 0 ||
              strcmp(type, "led_color") == 0) && _onPrimitive) {
    JsonObject action = doc.as<JsonObject>();
    _onPrimitive(action);

  } else if (strcmp(type, "telemetry") == 0 && _onTelemetryRequest) {
    _onTelemetryRequest();
  }
}

void RobotBLEServer::sendTelemetry(const String& json) {
  if (!_connected || !_pRxChar) return;
  _pRxChar->setValue(json.c_str());
  _pRxChar->notify();
}

bool RobotBLEServer::isBrainOnline() const {
  if (!_connected) return false;
  return (millis() - _lastHeartbeat) < HEARTBEAT_TIMEOUT_MS;
}

void RobotBLEServer::update() {
  // Detectar BRAIN_OFFLINE si hay conexión pero no heartbeat
  if (_connected && !isBrainOnline()) {
    static bool offlineLogged = false;
    if (!offlineLogged) {
      Serial.println("[BLE] BRAIN_OFFLINE — heartbeat perdido > 3s");
      offlineLogged = true;
    }
  } else {
    // Reset flag cuando el brain vuelve
  }
}