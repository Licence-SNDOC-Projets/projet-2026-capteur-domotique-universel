#include "ha_discovery.h"
#include "ha_factory.h"

// ⚠️ À ADAPTER À TON PROJET
extern void envoyerMessage(String mqtt_topic, String data);

// -----------------------------------------------------------------------------
// Configuration interne (stockée après begin())
// -----------------------------------------------------------------------------
namespace ha_discovery {

static const char* s_discoveryPrefix = "homeassistant";
static String s_manufacturer;
static String s_model;
static String s_swVersion;

// -----------------------------------------------------------------------------
// Initialisation (appelée UNE FOIS au boot)
// -----------------------------------------------------------------------------
void begin(const char* discoveryPrefix,
           const String& manufacturer,
           const String& model,
           const String& swVersion) {

  s_discoveryPrefix = discoveryPrefix;
  s_manufacturer    = manufacturer;
  s_model           = model;
  s_swVersion       = swVersion;
}

// -----------------------------------------------------------------------------
// Publication des entités Home Assistant (Discovery)
// Appelée au boot ET à chaque reconnexion MQTT
// -----------------------------------------------------------------------------
void publishDiscovery(String ip, String mac) {

  StaticJsonDocument<1024> doc;
  String topic, payload;

  // =====================================================================
  // ========== SENSOR : infos du module (IP, WiFi, etc.) =================
  // =====================================================================
  ha_factory::Options info;
  info.objectId    = "module_info";
  info.entityName  = "Infos Module";
  info.deviceName  = "Module " + mac;
  info.manufacturer = s_manufacturer;
  info.model        = s_model;
  info.swVersion    = s_swVersion;

  info.stateTopic        = "modules/" + mac + "/state";
  info.availabilityTopic = "modules/" + mac + "/availability";

  if (ha_factory::buildDiscoveryMessage(
        s_discoveryPrefix,
        HA_TYPE_SENSOR,
        mac,
        info,
        doc,
        topic,
        payload)) {

    // Ajout URL cliquable vers l’ESP32
    doc["device"]["configuration_url"] = "http://" + ip;

    payload = "";
    serializeJson(doc, payload);

    envoyerMessage(topic, payload); // retained
  }

  // =====================================================================
  // ========== BINARY SENSOR : état en ligne =============================
  // =====================================================================
  ha_factory::Options online;
  online.objectId    = "online";
  online.entityName  = "Module en ligne";
  online.deviceName = "Module " + mac;

  online.stateTopic = "modules/" + mac + "/availability";
  online.deviceClass = "connectivity";

  if (ha_factory::buildDiscoveryMessage(
        s_discoveryPrefix,
        HA_TYPE_BINARY_SENSOR,
        mac,
        online,
        doc,
        topic,
        payload)) {

    envoyerMessage(topic, payload);
  }

  // =====================================================================
  // ========== SWITCH : exemple relais ===================================
  // =====================================================================
  ha_factory::Options relay;
  relay.objectId    = "relay1";
  relay.entityName  = "Relais 1";
  relay.deviceName = "Module " + mac;

  relay.stateTopic   = "modules/" + mac + "/relay1/state";
  relay.commandTopic = "modules/" + mac + "/relay1/set";

  if (ha_factory::buildDiscoveryMessage(
        s_discoveryPrefix,
        HA_TYPE_SWITCH,
        mac,
        relay,
        doc,
        topic,
        payload)) {

    envoyerMessage(topic, payload);
  }
}

// -----------------------------------------------------------------------------
// Publication de l’état (appelée souvent : loop)
// -----------------------------------------------------------------------------
void publishState(const String& ip, const String& mac, float wifiRssi) {

  StaticJsonDocument<256> state;

  state["ip"] = ip;
  state["wifi_rssi"] = wifiRssi;

  String payload;
  serializeJson(state, payload);

  String topic = "modules/" + mac + "/state";
  envoyerMessage(topic, payload);
}

// -----------------------------------------------------------------------------
// Suppression d’une entité Home Assistant (optionnel)
// -----------------------------------------------------------------------------
void removeEntity(ha_equipment_type_t type,
                  const String& mac,
                  const String& objectId) {

  String uniqueId = mac;
  uniqueId.replace(":", "");
  uniqueId.toLowerCase();
  uniqueId += "_" + objectId;

  String topic =
    String(s_discoveryPrefix) + "/" +
    ha_type_to_string(type) + "/" +
    uniqueId + "/config";

  // Payload vide + retained = suppression HA
  envoyerMessage(topic, "");
}

} // namespace ha_discovery
