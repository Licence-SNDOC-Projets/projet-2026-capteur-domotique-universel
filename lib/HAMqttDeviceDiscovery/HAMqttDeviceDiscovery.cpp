#include "HAMqttDeviceDiscovery.h"
#include <ArduinoJson.h>

HAMqttDeviceDiscovery::HAMqttDeviceDiscovery(PubSubClient& mqttClient)
  : _mqtt(mqttClient) {}

void HAMqttDeviceDiscovery::begin(const DeviceInfo& device,
                                  const OriginInfo& origin,
                                  const String& discoveryPrefix) {
  _device = device;
  _origin = origin;
  _discoveryPrefix = discoveryPrefix.length() ? discoveryPrefix : "homeassistant";

  _device.ids = sanitizeId(_device.ids);

  subscribeTopics();
}

void HAMqttDeviceDiscovery::setAvailability(const String& topic,
                                            const String& payloadAvailable,
                                            const String& payloadNotAvailable) {
  _availabilityTopic = topic;
  _payloadAvailable = payloadAvailable;
  _payloadNotAvailable = payloadNotAvailable;
}

void HAMqttDeviceDiscovery::setBirthTopic(const String& topic, const String& onlinePayload) {
  _birthTopic = topic;
  _birthOnlinePayload = onlinePayload;
}

void HAMqttDeviceDiscovery::setSwitchCommandCallback(SwitchCommandCallback callback) {
  _switchCallback = callback;
}

bool HAMqttDeviceDiscovery::addSensor(const String& componentId,
                                      const String& uniqueId,
                                      const String& stateTopic,
                                      const String& deviceClass,
                                      const String& unit,
                                      const String& stateClass,
                                      const String& valueTemplate,
                                      const String& name,
                                      const String& objectId,
                                      const String& icon) {
  int idx = findComponentIndex(componentId);
  if (idx < 0) idx = findFreeSlot();
  if (idx < 0) return false;

  Component& c = _components[idx];
  c.used = true;
  c.type = SENSOR;
  c.id = sanitizeId(componentId);
  c.unique_id = uniqueId;
  c.state_topic = stateTopic;
  c.device_class = deviceClass;
  c.unit_of_measurement = unit;
  c.state_class = stateClass;
  c.value_template = valueTemplate;
  c.name = name;
  c.object_id = objectId;
  c.icon = icon;
  return true;
}

bool HAMqttDeviceDiscovery::addBinarySensor(const String& componentId,
                                            const String& uniqueId,
                                            const String& stateTopic,
                                            const String& deviceClass,
                                            const String& payloadOn,
                                            const String& payloadOff,
                                            const String& valueTemplate,
                                            const String& name,
                                            const String& objectId,
                                            const String& icon) {
  int idx = findComponentIndex(componentId);
  if (idx < 0) idx = findFreeSlot();
  if (idx < 0) return false;

  Component& c = _components[idx];
  c.used = true;
  c.type = BINARY_SENSOR;
  c.id = sanitizeId(componentId);
  c.unique_id = uniqueId;
  c.state_topic = stateTopic;
  c.device_class = deviceClass;
  c.payload_on = payloadOn;
  c.payload_off = payloadOff;
  c.value_template = valueTemplate;
  c.name = name;
  c.object_id = objectId;
  c.icon = icon;
  return true;
}

bool HAMqttDeviceDiscovery::addSwitch(const String& componentId,
                                      const String& uniqueId,
                                      const String& stateTopic,
                                      const String& commandTopic,
                                      const String& payloadOn,
                                      const String& payloadOff,
                                      const String& name,
                                      const String& objectId,
                                      const String& icon,
                                      bool optimistic) {
  int idx = findComponentIndex(componentId);
  if (idx < 0) idx = findFreeSlot();
  if (idx < 0) return false;

  Component& c = _components[idx];
  c.used = true;
  c.type = SWITCH;
  c.id = sanitizeId(componentId);
  c.unique_id = uniqueId;
  c.state_topic = stateTopic;
  c.command_topic = commandTopic;
  c.payload_on = payloadOn;
  c.payload_off = payloadOff;
  c.name = name;
  c.object_id = objectId;
  c.icon = icon;
  c.optimistic = optimistic;
  return true;
}

bool HAMqttDeviceDiscovery::publishDiscovery() {
  if (!_mqtt.connected()) return false;
  if (_device.ids.isEmpty() || _device.name.isEmpty()) return false;
  if (_origin.name.isEmpty()) return false;

  StaticJsonDocument<4096> doc;

  JsonObject dev = doc.createNestedObject("dev");
  dev["ids"] = _device.ids;
  dev["name"] = _device.name;
  if (!_device.mf.isEmpty()) dev["mf"] = _device.mf;
  if (!_device.mdl.isEmpty()) dev["mdl"] = _device.mdl;
  if (!_device.mdl_id.isEmpty()) dev["mdl_id"] = _device.mdl_id;
  if (!_device.sw.isEmpty()) dev["sw"] = _device.sw;
  if (!_device.sn.isEmpty()) dev["sn"] = _device.sn;
  if (!_device.hw.isEmpty()) dev["hw"] = _device.hw;
  if (!_device.cu.isEmpty()) dev["cu"] = _device.cu;

  JsonObject origin = doc.createNestedObject("o");
  origin["name"] = _origin.name;
  if (!_origin.sw.isEmpty()) origin["sw"] = _origin.sw;
  if (!_origin.url.isEmpty()) origin["url"] = _origin.url;

  if (!_availabilityTopic.isEmpty()) {
    doc["avty_t"] = _availabilityTopic;
    doc["pl_avail"] = _payloadAvailable;
    doc["pl_not_avail"] = _payloadNotAvailable;
  }

  JsonObject cmps = doc.createNestedObject("cmps");

  for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
    if (!_components[i].used) continue;

    const Component& c = _components[i];
    JsonObject item = cmps.createNestedObject(c.id);

    switch (c.type) {
      case SENSOR:
        item["p"] = "sensor";
        break;
      case BINARY_SENSOR:
        item["p"] = "binary_sensor";
        break;
      case SWITCH:
        item["p"] = "switch";
        break;
    }

    item["unique_id"] = c.unique_id;
    item["state_topic"] = c.state_topic;

    if (!c.name.isEmpty()) item["name"] = c.name;
    if (!c.object_id.isEmpty()) item["object_id"] = c.object_id;
    if (!c.icon.isEmpty()) item["icon"] = c.icon;
    if (!c.device_class.isEmpty()) item["device_class"] = c.device_class;
    if (!c.unit_of_measurement.isEmpty()) item["unit_of_measurement"] = c.unit_of_measurement;
    if (!c.state_class.isEmpty()) item["state_class"] = c.state_class;
    if (!c.value_template.isEmpty()) item["value_template"] = c.value_template;

    if (c.type == BINARY_SENSOR) {
      item["payload_on"] = c.payload_on;
      item["payload_off"] = c.payload_off;
    }

    if (c.type == SWITCH) {
      item["command_topic"] = c.command_topic;
      item["payload_on"] = c.payload_on;
      item["payload_off"] = c.payload_off;
      if (c.optimistic) item["optimistic"] = true;
    }
  }

  String payload;
  serializeJson(doc, payload);
  return publishRaw(getDiscoveryTopic(), payload, true);
}

bool HAMqttDeviceDiscovery::publishAvailability(bool online, bool retain) {
  if (_availabilityTopic.isEmpty()) return false;
  return publishRaw(
    _availabilityTopic,
    online ? _payloadAvailable : _payloadNotAvailable,
    retain
  );
}

bool HAMqttDeviceDiscovery::publishTopicState(const String& topic, const String& payload, bool retain) {
  return publishRaw(topic, payload, retain);
}

bool HAMqttDeviceDiscovery::publishComponentState(const String& componentId, const String& payload, bool retain) {
  int idx = findComponentIndex(componentId);
  if (idx < 0) return false;
  return publishRaw(_components[idx].state_topic, payload, retain);
}

bool HAMqttDeviceDiscovery::handleMessage(const String& topic, const String& payload) {
  if (topic == _birthTopic && payload == _birthOnlinePayload) {
    publishDiscovery();
    publishAvailability(true, true);
    return true;
  }

  for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
    if (!_components[i].used) continue;

    Component& c = _components[i];
    if (c.type == SWITCH && topic == c.command_topic) {
      if (_switchCallback) {
        _switchCallback(c.id, payload);
      }
      return true;
    }
  }

  return false;
}

bool HAMqttDeviceDiscovery::handleMessage(char* topic, byte* payload, unsigned int length) {
  String t = topic ? String(topic) : "";
  String p;
  p.reserve(length);

  for (unsigned int i = 0; i < length; i++) {
    p += (char)payload[i];
  }

  return handleMessage(t, p);
}

void HAMqttDeviceDiscovery::loop() {
  subscribeTopics();
}

int HAMqttDeviceDiscovery::findFreeSlot() const {
  for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
    if (!_components[i].used) return i;
  }
  return -1;
}

int HAMqttDeviceDiscovery::findComponentIndex(const String& componentId) const {
  String id = sanitizeId(componentId);
  for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
    if (_components[i].used && _components[i].id == id) return i;
  }
  return -1;
}

String HAMqttDeviceDiscovery::sanitizeId(const String& input) const {
  String out;
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    bool valid =
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '_' || c == '-';

    if (valid) {
      out += (char)tolower(c);
    } else if (c == ' ' || c == '.' || c == '/' || c == ':') {
      out += '_';
    }
  }

  while (out.indexOf("__") >= 0) {
    out.replace("__", "_");
  }

  if (out.isEmpty()) out = "item";
  return out;
}

String HAMqttDeviceDiscovery::getDiscoveryTopic() const {
  return _discoveryPrefix + "/device/" + _device.ids + "/config";
}

bool HAMqttDeviceDiscovery::publishRaw(const String& topic, const String& payload, bool retain) {
  if (!_mqtt.connected()) return false;
  return _mqtt.publish(topic.c_str(), payload.c_str(), retain);
}

void HAMqttDeviceDiscovery::subscribeTopics() {
  if (!_mqtt.connected()) return;

  if (!_birthTopic.isEmpty()) {
    _mqtt.subscribe(_birthTopic.c_str());
  }

  for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
    if (_components[i].used &&
        _components[i].type == SWITCH &&
        !_components[i].command_topic.isEmpty()) {
      _mqtt.subscribe(_components[i].command_topic.c_str());
    }
  }
}