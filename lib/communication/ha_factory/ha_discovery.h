#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ha_equipment_types.h"
#include "ha_factory.h"

// -----------------------------------------------------------------------------
// Home Assistant Discovery (project-specific)
// - This module decides WHICH entities your device exposes to Home Assistant
// - It uses ha_factory to BUILD discovery messages (topic + payload)
// - Your mqtt layer should provide envoyerMessage(topic, payload, retained)
// -----------------------------------------------------------------------------

namespace ha_discovery {

    void begin(const char* discoveryPrefix,
            const String& manufacturer,
            const String& model,
            const String& swVersion);

    void publishDiscovery(const String& ip, const String& mac);
    void publishState(const String& ip, const String& mac, float wifiRssi);
    void removeEntity(ha_equipment_type_t type, const String& mac, const String& objectId);

}