#ifndef HA_MQTT_DEVICE_DISCOVERY_H
#define HA_MQTT_DEVICE_DISCOVERY_H

#include <Arduino.h>
#include <PubSubClient.h>

#ifndef HA_MAX_COMPONENTS
#define HA_MAX_COMPONENTS 20
#endif

class HAMqttDeviceDiscovery {
public:
  enum ComponentType {
    SENSOR,
    BINARY_SENSOR,
    SWITCH
  };

  typedef void (*SwitchCommandCallback)(const String& componentId, const String& payload);

  struct DeviceInfo {
    String ids;      // obligatoire dans dev
    String name;     // obligatoire dans dev
    String mf;       // manufacturer
    String mdl;      // model
    String mdl_id;   // model_id
    String sw;       // sw_version
    String sn;       // serial_number
    String hw;       // hw_version
    String cu;       // configuration_url
  };

  struct OriginInfo {
    String name;     // obligatoire dans o
    String sw;       // sw_version
    String url;      // support_url
  };

  struct Component {
    bool used = false;
    ComponentType type = SENSOR;

    String id;             // clé dans cmps
    String name;           // optionnel
    String unique_id;      // obligatoire pour entités
    String object_id;      // optionnel mais utile

    String state_topic;
    String command_topic;

    String device_class;
    String unit_of_measurement;
    String state_class;
    String value_template;
    String icon;

    String payload_on = "ON";
    String payload_off = "OFF";

    bool optimistic = false;
    bool retain_state = true;
  };

  explicit HAMqttDeviceDiscovery(PubSubClient& mqttClient);

  void begin(const DeviceInfo& device,
             const OriginInfo& origin,
             const String& discoveryPrefix = "homeassistant");

  void setAvailability(const String& topic,
                       const String& payloadAvailable = "online",
                       const String& payloadNotAvailable = "offline");

  void setBirthTopic(const String& topic = "homeassistant/status",
                     const String& onlinePayload = "online");

  void setSwitchCommandCallback(SwitchCommandCallback callback);

  bool addSensor(const String& componentId,
                 const String& uniqueId,
                 const String& stateTopic,
                 const String& deviceClass = "",
                 const String& unit = "",
                 const String& stateClass = "",
                 const String& valueTemplate = "",
                 const String& name = "",
                 const String& objectId = "",
                 const String& icon = "");

  bool addBinarySensor(const String& componentId,
                       const String& uniqueId,
                       const String& stateTopic,
                       const String& deviceClass = "",
                       const String& payloadOn = "ON",
                       const String& payloadOff = "OFF",
                       const String& valueTemplate = "",
                       const String& name = "",
                       const String& objectId = "",
                       const String& icon = "");

  bool addSwitch(const String& componentId,
                 const String& uniqueId,
                 const String& stateTopic,
                 const String& commandTopic,
                 const String& payloadOn = "ON",
                 const String& payloadOff = "OFF",
                 const String& name = "",
                 const String& objectId = "",
                 const String& icon = "",
                 bool optimistic = false);

  bool publishDiscovery();
  bool publishAvailability(bool online, bool retain = true);

  bool publishTopicState(const String& topic, const String& payload, bool retain = true);
  bool publishComponentState(const String& componentId, const String& payload, bool retain = true);

  bool handleMessage(const String& topic, const String& payload);
  bool handleMessage(char* topic, byte* payload, unsigned int length);

  void loop();

private:
  PubSubClient& _mqtt;
  DeviceInfo _device;
  OriginInfo _origin;
  Component _components[HA_MAX_COMPONENTS];

  String _discoveryPrefix = "homeassistant";
  String _birthTopic = "homeassistant/status";
  String _birthOnlinePayload = "online";

  String _availabilityTopic = "";
  String _payloadAvailable = "online";
  String _payloadNotAvailable = "offline";

  SwitchCommandCallback _switchCallback = nullptr;

  int findFreeSlot() const;
  int findComponentIndex(const String& componentId) const;

  String sanitizeId(const String& input) const;
  String getDiscoveryTopic() const;

  bool publishRaw(const String& topic, const String& payload, bool retain);
  void subscribeTopics();
};

#endif