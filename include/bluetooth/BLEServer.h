#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "config.h"

// Callbacks de comandos recibidos
typedef std::function<void(JsonObject& action)>                    PrimitiveCallback;
typedef std::function<void()>                                      StopCallback;
typedef std::function<void(JsonArray& steps)>                      SequenceCallback;
typedef std::function<void()>                                      TelemetryRequestCallback;

class RobotBLEServer : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
  void begin();
  bool isConnected() const { return _connected; }
  void sendTelemetry(const String& json);
  void registerCallbacks(PrimitiveCallback        onPrimitive,
                         StopCallback             onStop,
                         SequenceCallback         onSequence,
                         TelemetryRequestCallback onTelemetryRequest = nullptr);
  // Llamar periódicamente
  void update();
  bool isBrainOnline() const;

  // BLEServerCallbacks
  void onConnect(BLEServer* server)    override;
  void onDisconnect(BLEServer* server) override;

  // BLECharacteristicCallbacks
  void onWrite(BLECharacteristic* characteristic) override;

private:
  bool              _connected      = false;
  unsigned long     _lastHeartbeat  = 0;
  BLECharacteristic* _pRxChar       = nullptr;
  PrimitiveCallback _onPrimitive;
  StopCallback      _onStop;
  SequenceCallback  _onSequence;
  TelemetryRequestCallback _onTelemetryRequest;

  void handleCommand(const String& json);
};

extern RobotBLEServer bleServer;