#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ha_equipment_types.h"   // ton fichier .h

// ------------------------------------------------------------
// Home Assistant Discovery Factory
// ------------------------------------------------------------
namespace ha_factory {

// Petit struct pour passer des options sans rendre la fonction illisible
struct Options {
  String deviceName        = "test_esp32";       // si vide -> "Module <mac>"
  String manufacturer      = "SN_DOC";
  String model             = "ESP32";
  String swVersion         = "1.0.0";

  String entityName        = "";       // nom affiché dans HA (friendly name)
  String objectId          = "";       // slug stable: ex "relay1", "temp_salon"
  String stateTopic        = "";       // si vide -> "modules/<mac>/<objectId>/state"
  String commandTopic      = "";       // utile switch/light/cover/etc (si vide -> auto)
  String availabilityTopic = "";       // si vide -> "modules/<mac>/availability"

  // sensor-specific (facultatif)
  String unit              = "";       // "°C", "%", "W", etc.
  String deviceClass       = "";       // "temperature", "humidity", "power", etc.
  String stateClass        = "";       // "measurement", "total_increasing", etc.
  int    expireAfterSec    = 0;        // 0 = pas utilisé
};

// Fabrique le topic discovery: homeassistant/<platform>/<unique_id>/config
static inline String makeDiscoveryTopic(const char* discoveryPrefix,
                                       ha_equipment_type_t type,
                                       const String& uniqueId) {
  return String(discoveryPrefix) + "/" + ha_type_to_string(type) + "/" + uniqueId + "/config";
}

// Fabrique un unique_id stable (évite les ":" qui posent parfois problème)
static inline String makeUniqueId(const String& mac, const String& objectId) {
  String sanitized = mac;
  sanitized.replace(":", "");
  sanitized.toLowerCase();
  return sanitized + "_" + objectId;
}

// Remplit le bloc "device" (crée le module côté HA)
static inline void fillDeviceBlock(JsonDocument& doc,
                                   const String& mac,
                                   const Options& opt) {
  JsonObject device = doc.createNestedObject("device");

  // identifiers: clé de regroupement des entités dans le même "module"
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(mac);

  device["name"]         = opt.deviceName.length() ? opt.deviceName : ("Module " + mac);
  device["manufacturer"] = opt.manufacturer;
  device["model"]        = opt.model;
  device["sw_version"]   = opt.swVersion;

  // Optionnel mais sympa: connections (affiche la MAC)
  JsonArray connections = device.createNestedArray("connections");
  JsonArray c0 = connections.createNestedArray();
  c0.add("mac");
  c0.add(mac);
}

// Point central: construit le payload JSON pour un type donné
static inline bool buildDiscoveryPayload(JsonDocument& doc,
                                        ha_equipment_type_t type,
                                        const String& mac,
                                        const Options& opt,
                                        String& outUniqueId) {
  if (!opt.objectId.length()) return false; // objectId obligatoire pour un unique_id stable

  doc.clear();

  // Base topics par défaut
  const String base = "modules/" + mac + "/" + opt.objectId;
  const String stateTopic        = opt.stateTopic.length() ? opt.stateTopic : (base + "/state");
  const String availabilityTopic = opt.availabilityTopic.length()
                                 ? opt.availabilityTopic
                                 : ("modules/" + mac + "/availability");

  outUniqueId = makeUniqueId(mac, opt.objectId);

  // Champs communs
  doc["name"]               = opt.entityName.length() ? opt.entityName : opt.objectId;
  doc["unique_id"]          = outUniqueId;
  doc["state_topic"]        = stateTopic;
  doc["availability_topic"] = availabilityTopic;

  // Crée le device (= module)
  fillDeviceBlock(doc, mac, opt);

  // Champs spécifiques selon le type
  switch (type) {

    case HA_TYPE_SENSOR: {
      // sensor: unit/device_class/state_class optionnels
      if (opt.unit.length())        doc["unit_of_measurement"] = opt.unit;
      if (opt.deviceClass.length()) doc["device_class"] = opt.deviceClass;
      if (opt.stateClass.length())  doc["state_class"]  = opt.stateClass;
      if (opt.expireAfterSec > 0)   doc["expire_after"] = opt.expireAfterSec;
      break;
    }

    case HA_TYPE_BINARY_SENSOR: {
      // souvent utile: device_class (motion, door, window, smoke, etc.)
      if (opt.deviceClass.length()) doc["device_class"] = opt.deviceClass;
      if (opt.expireAfterSec > 0)   doc["expire_after"] = opt.expireAfterSec;
      // payload_on/off optionnels (HA a des defaults, mais tu peux forcer)
      // doc["payload_on"] = "1"; doc["payload_off"] = "0";
      break;
    }

    case HA_TYPE_SWITCH: {
      const String cmd = opt.commandTopic.length() ? opt.commandTopic : (base + "/set");
      doc["command_topic"] = cmd;
      doc["payload_on"]  = "ON";
      doc["payload_off"] = "OFF";
      // Optimiste = met à jour l’état sans retour (si tu veux)
      // doc["optimistic"] = true;
      break;
    }

    case HA_TYPE_LIGHT: {
      const String cmd = opt.commandTopic.length() ? opt.commandTopic : (base + "/set");
      doc["command_topic"] = cmd;
      doc["payload_on"]  = "ON";
      doc["payload_off"] = "OFF";
      // Si tu gères brightness/rgb, tu ajoutes ici (schema_json, brightness, etc.)
      break;
    }

    case HA_TYPE_COVER: {
      const String cmd = opt.commandTopic.length() ? opt.commandTopic : (base + "/set");
      doc["command_topic"] = cmd;
      // Selon ton implémentation tu peux utiliser open/close/stop
      // doc["payload_open"] = "OPEN";
      // doc["payload_close"] = "CLOSE";
      // doc["payload_stop"] = "STOP";
      break;
    }

    default:
      // Type non encore géré -> on garde au moins name/unique_id/state_topic/device
      break;
  }

  return true;
}

// Helper "tout-en-un": retourne topic + payload JSON sérialisé
static inline bool buildDiscoveryMessage(const char* discoveryPrefix,
                                        ha_equipment_type_t type,
                                        const String& mac,
                                        const Options& opt,
                                        JsonDocument& doc,
                                        String& outTopic,
                                        String& outPayload) {
  String uniqueId;
  if (!buildDiscoveryPayload(doc, type, mac, opt, uniqueId)) return false;

  outTopic = makeDiscoveryTopic(discoveryPrefix, type, uniqueId);

  outPayload = "";
  serializeJson(doc, outPayload);
  return true;
}

} // namespace ha_factory
