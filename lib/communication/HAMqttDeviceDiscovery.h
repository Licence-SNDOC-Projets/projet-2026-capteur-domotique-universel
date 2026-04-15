#ifndef HA_MQTT_DEVICE_DISCOVERY_H
#define HA_MQTT_DEVICE_DISCOVERY_H

#include <Arduino.h>
#include "communication.h"
#include "HAMqttTypes.h"

#ifndef HA_MAX_COMPONENTS
#define HA_MAX_COMPONENTS 20
#endif

class HAMqttDeviceDiscovery : public Communication
{
public:
    typedef void (*SwitchCommandCallback)(const String& componentId, const String& payload);

    struct DeviceInfo
    {
        String ids;
        String name;
        String mf;
        String mdl;
        String mdl_id;
        String sw;
        String sn;
        String hw;
        String cu;
    };

    struct OriginInfo
    {
        String name;
        String sw;
        String url;
    };

    struct Component
    {
        bool used = false;

        HAComponentType type = HAComponentType::SENSOR;
        String platform = "sensor";

        String id;
        String name;
        String unique_id;
        String object_id;

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

public:
    explicit HAMqttDeviceDiscovery(PubSubClient& mqttClient);
    virtual ~HAMqttDeviceDiscovery();

    void begin(const DeviceInfo& device,
               const OriginInfo& origin,
               const String& discoveryPrefix = "homeassistant");

    void setAvailability(const String& topic,
                         const String& payloadAvailable = "online",
                         const String& payloadNotAvailable = "offline");

    void setBirthTopic(const String& topic = "homeassistant/status",
                       const String& onlinePayload = "online");

    void setSwitchCommandCallback(SwitchCommandCallback callback);

    bool addComponent(const String& componentId,
                      HAComponentType type,
                      const String& uniqueId,
                      const String& stateTopic = "",
                      const String& commandTopic = "",
                      const String& name = "",
                      const String& objectId = "",
                      const String& deviceClass = "",
                      const String& unit = "",
                      const String& stateClass = "",
                      const String& valueTemplate = "",
                      const String& icon = "",
                      const String& payloadOn = "ON",
                      const String& payloadOff = "OFF",
                      bool optimistic = false,
                      bool retainState = true);

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

private:
    int findFreeSlot() const;
    int findComponentIndex(const String& componentId) const;

    String sanitizeId(const String& input) const;
    String getDiscoveryTopic() const;

    bool publishRaw(const String& topic, const String& payload, bool retain);
    void subscribeTopics();
};

#endif