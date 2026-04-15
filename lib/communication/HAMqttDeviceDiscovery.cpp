#include "HAMqttDeviceDiscovery.h"
#include <ArduinoJson.h>
#include <ctype.h>

HAMqttDeviceDiscovery::HAMqttDeviceDiscovery(PubSubClient& mqttClient)
    : Communication(mqttClient)
{
}

HAMqttDeviceDiscovery::~HAMqttDeviceDiscovery()
{
}

void HAMqttDeviceDiscovery::begin(const DeviceInfo& device,
                                  const OriginInfo& origin,
                                  const String& discoveryPrefix)
{
    _device = device;
    _origin = origin;
    _discoveryPrefix = discoveryPrefix;
}

void HAMqttDeviceDiscovery::setAvailability(const String& topic,
                                            const String& payloadAvailable,
                                            const String& payloadNotAvailable)
{
    _availabilityTopic = topic;
    _payloadAvailable = payloadAvailable;
    _payloadNotAvailable = payloadNotAvailable;
}

void HAMqttDeviceDiscovery::setBirthTopic(const String& topic,
                                          const String& onlinePayload)
{
    _birthTopic = topic;
    _birthOnlinePayload = onlinePayload;
}

void HAMqttDeviceDiscovery::setSwitchCommandCallback(SwitchCommandCallback callback)
{
    _switchCallback = callback;
}

bool HAMqttDeviceDiscovery::addComponent(const String& componentId,
                                         HAComponentType type,
                                         const String& uniqueId,
                                         const String& stateTopic,
                                         const String& commandTopic,
                                         const String& name,
                                         const String& objectId,
                                         const String& deviceClass,
                                         const String& unit,
                                         const String& stateClass,
                                         const String& valueTemplate,
                                         const String& icon,
                                         const String& payloadOn,
                                         const String& payloadOff,
                                         bool optimistic,
                                         bool retainState)
{
    int index = findComponentIndex(componentId);

    if (index < 0) {
        index = findFreeSlot();
    }

    if (index < 0) {
        return false;
    }

    Component& component = _components[index];
    component.used = true;

    component.type = type;
    component.platform = HAComponentTypeToString(type);

    component.id = sanitizeId(componentId);
    component.name = name;
    component.unique_id = uniqueId;
    component.object_id = objectId;

    component.state_topic = stateTopic;
    component.command_topic = commandTopic;

    component.device_class = deviceClass;
    component.unit_of_measurement = unit;
    component.state_class = stateClass;
    component.value_template = valueTemplate;
    component.icon = icon;

    component.payload_on = payloadOn;
    component.payload_off = payloadOff;

    component.optimistic = optimistic;
    component.retain_state = retainState;

    return true;
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
                                      const String& icon)
{
    return addComponent(componentId,
                        HAComponentType::SENSOR,
                        uniqueId,
                        stateTopic,
                        "",
                        name,
                        objectId,
                        deviceClass,
                        unit,
                        stateClass,
                        valueTemplate,
                        icon);
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
                                            const String& icon)
{
    return addComponent(componentId,
                        HAComponentType::BINARY_SENSOR,
                        uniqueId,
                        stateTopic,
                        "",
                        name,
                        objectId,
                        deviceClass,
                        "",
                        "",
                        valueTemplate,
                        icon,
                        payloadOn,
                        payloadOff);
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
                                      bool optimistic)
{
    return addComponent(componentId,
                        HAComponentType::SWITCH,
                        uniqueId,
                        stateTopic,
                        commandTopic,
                        name,
                        objectId,
                        "",
                        "",
                        "",
                        "",
                        icon,
                        payloadOn,
                        payloadOff,
                        optimistic);
}

bool HAMqttDeviceDiscovery::publishDiscovery()
{
    if (!getEtatMqtt()) {
        return false;
    }

    DynamicJsonDocument doc(8192);

    JsonObject dev = doc.createNestedObject("dev");
    dev["ids"] = _device.ids;
    dev["name"] = _device.name;

    if (_device.mf.length())     dev["mf"] = _device.mf;
    if (_device.mdl.length())    dev["mdl"] = _device.mdl;
    if (_device.mdl_id.length()) dev["mdl_id"] = _device.mdl_id;
    if (_device.sw.length())     dev["sw"] = _device.sw;
    if (_device.sn.length())     dev["sn"] = _device.sn;
    if (_device.hw.length())     dev["hw"] = _device.hw;
    if (_device.cu.length())     dev["cu"] = _device.cu;

    JsonObject origin = doc.createNestedObject("o");
    if (_origin.name.length()) origin["name"] = _origin.name;
    if (_origin.sw.length())   origin["sw"] = _origin.sw;
    if (_origin.url.length())  origin["url"] = _origin.url;

    if (_availabilityTopic.length()) {
        JsonArray avty = doc.createNestedArray("avty");
        JsonObject av = avty.createNestedObject();
        av["t"] = _availabilityTopic;
        av["pl_avail"] = _payloadAvailable;
        av["pl_not_avail"] = _payloadNotAvailable;
    }

    JsonObject cmps = doc.createNestedObject("cmps");

    for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
        if (!_components[i].used) {
            continue;
        }

        const Component& component = _components[i];
        JsonObject cmp = cmps.createNestedObject(component.id);

        cmp["p"] = component.platform;

        if (component.name.length())                cmp["name"] = component.name;
        if (component.unique_id.length())           cmp["unique_id"] = component.unique_id;
        if (component.object_id.length())           cmp["object_id"] = component.object_id;
        if (component.state_topic.length())         cmp["state_topic"] = component.state_topic;
        if (component.command_topic.length())       cmp["command_topic"] = component.command_topic;
        if (component.device_class.length())        cmp["device_class"] = component.device_class;
        if (component.unit_of_measurement.length()) cmp["unit_of_measurement"] = component.unit_of_measurement;
        if (component.state_class.length())         cmp["state_class"] = component.state_class;
        if (component.value_template.length())      cmp["value_template"] = component.value_template;
        if (component.icon.length())                cmp["icon"] = component.icon;

        if (component.type == HAComponentType::BINARY_SENSOR ||
            component.type == HAComponentType::SWITCH) {
            cmp["payload_on"] = component.payload_on;
            cmp["payload_off"] = component.payload_off;
        }

        if (component.type == HAComponentType::SWITCH) {
            cmp["optimistic"] = component.optimistic;
        }
    }

    String payload;
    serializeJson(doc, payload);

    return publishRaw(getDiscoveryTopic(), payload, true);
}

bool HAMqttDeviceDiscovery::publishAvailability(bool online, bool retain)
{
    if (_availabilityTopic.isEmpty()) {
        return false;
    }

    return publishRaw(_availabilityTopic,
                      online ? _payloadAvailable : _payloadNotAvailable,
                      retain);
}

bool HAMqttDeviceDiscovery::publishTopicState(const String& topic,
                                              const String& payload,
                                              bool retain)
{
    if (topic.isEmpty()) {
        return false;
    }

    return publishRaw(topic, payload, retain);
}

bool HAMqttDeviceDiscovery::publishComponentState(const String& componentId,
                                                  const String& payload,
                                                  bool retain)
{
    int index = findComponentIndex(componentId);
    if (index < 0) {
        return false;
    }

    const Component& component = _components[index];

    if (component.state_topic.isEmpty()) {
        return false;
    }

    return publishRaw(component.state_topic, payload, retain);
}

bool HAMqttDeviceDiscovery::handleMessage(const String& topic, const String& payload)
{
    if (topic == _birthTopic && payload == _birthOnlinePayload) {
        publishDiscovery();
        subscribeTopics();

        if (_availabilityTopic.length()) {
            publishAvailability(true);
        }

        return true;
    }

    for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
        if (!_components[i].used) {
            continue;
        }

        const Component& component = _components[i];

        if (component.command_topic.length() && topic == component.command_topic) {
            if (_switchCallback != nullptr) {
                _switchCallback(component.id, payload);
            }
            return true;
        }
    }

    return false;
}

bool HAMqttDeviceDiscovery::handleMessage(char* topic, byte* payload, unsigned int length)
{
    String topicStr = String(topic);
    String payloadStr;
    payloadStr.reserve(length);

    for (unsigned int i = 0; i < length; i++) {
        payloadStr += static_cast<char>(payload[i]);
    }

    return handleMessage(topicStr, payloadStr);
}

void HAMqttDeviceDiscovery::loop()
{
    receptionDataMQTT();
}

int HAMqttDeviceDiscovery::findFreeSlot() const
{
    for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
        if (!_components[i].used) {
            return i;
        }
    }

    return -1;
}

int HAMqttDeviceDiscovery::findComponentIndex(const String& componentId) const
{
    String cleanId = sanitizeId(componentId);

    for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
        if (_components[i].used && _components[i].id == cleanId) {
            return i;
        }
    }

    return -1;
}

String HAMqttDeviceDiscovery::sanitizeId(const String& input) const
{
    String result;
    result.reserve(input.length());

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        if (isalnum(static_cast<unsigned char>(c))) {
            result += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        } else if (c == '_' || c == '-') {
            result += c;
        } else if (c == ' ' || c == '/' || c == '.') {
            result += '_';
        }
    }

    return result;
}

String HAMqttDeviceDiscovery::getDiscoveryTopic() const
{
    return _discoveryPrefix + "/device/" + sanitizeId(_device.ids) + "/config";
}

bool HAMqttDeviceDiscovery::publishRaw(const String& topic,
                                       const String& payload,
                                       bool retain)
{
    if (!getEtatMqtt()) {
        return false;
    }

    return getMqttClient().publish(topic.c_str(), payload.c_str(), retain);
}

void HAMqttDeviceDiscovery::subscribeTopics()
{
    if (!getEtatMqtt()) {
        return;
    }

    if (_birthTopic.length()) {
        getMqttClient().subscribe(_birthTopic.c_str());
    }

    for (int i = 0; i < HA_MAX_COMPONENTS; i++) {
        if (!_components[i].used) {
            continue;
        }

        if (_components[i].command_topic.length()) {
            getMqttClient().subscribe(_components[i].command_topic.c_str());
        }
    }
}