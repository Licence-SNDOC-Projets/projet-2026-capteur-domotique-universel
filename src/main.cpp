#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "HAMqttDeviceDiscovery.h"

// ================= WIFI =================
const char* WIFI_SSID = "TP-Link_5270";
const char* WIFI_PASS = "25968460";

// ================= MQTT =================
const char* MQTT_HOST = "192.168.0.105";
const int   MQTT_PORT = 1883;
const char* MQTT_USER = "sndoc";
const char* MQTT_PASS = "sndoc";

// ================= GPIO =================
#define RELAY_PIN 2

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
HAMqttDeviceDiscovery ha(mqttClient);

bool relayState = false;
float temperatureValue = 21.5;
bool motionDetected = false;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  ha.handleMessage(topic, payload, length);
}

void onSwitchCommand(const String& componentId, const String& payload) {
  Serial.print("Commande recue pour: ");
  Serial.println(componentId);

  Serial.print("Payload recu: ");
  Serial.println(payload);

  if (componentId == "relay_1") {
    if (payload == "ON") {
      relayState = true;
      digitalWrite(RELAY_PIN, HIGH);
    } else if (payload == "OFF") {
      relayState = false;
      digitalWrite(RELAY_PIN, LOW);
    }

    ha.publishTopicState(
      "devices/esp32_salon/relay_1/state",
      relayState ? "ON" : "OFF",
      true
    );
  }
}

void connectWiFi() {
  Serial.println("Connexion WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connecte");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  mqttClient.setBufferSize(4096);

  ha.setSwitchCommandCallback(onSwitchCommand);

  while (!mqttClient.connected()) {
    String clientId = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.print("Connexion MQTT... ");
    if (mqttClient.connect(
          clientId.c_str(),
          MQTT_USER,
          MQTT_PASS,
          "devices/esp32_salon/availability",
          1,
          true,
          "offline")) {

      Serial.println("OK");

      Serial.println("Publication availability...");
      bool ok1 = ha.publishAvailability(true, true);
      Serial.println(ok1 ? "OK" : "ECHEC");

      Serial.println("Publication discovery...");
      Serial.println("Topic attendu: homeassistant/device/esp32_salon/config");
      bool ok2 = ha.publishDiscovery();
      Serial.println(ok2 ? "OK" : "ECHEC");

    } else {
      Serial.print("Erreur MQTT rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  connectWiFi();

  HAMqttDeviceDiscovery::DeviceInfo dev;
  dev.ids = "esp32_salon";
  dev.name = "ESP32 Salon";
  dev.mf = "DIY";
  dev.mdl = "ESP32 DevKit";
  dev.sw = "1.0.0";
  dev.sn = "esp32_salon_001";
  dev.hw = "revA";

  HAMqttDeviceDiscovery::OriginInfo origin;
  origin.name = "esp32-ha-discovery";
  origin.sw = "1.0.0";
  origin.url = "https://example.local/support";

  ha.begin(dev, origin, "homeassistant");
  ha.setAvailability("devices/esp32_salon/availability", "online", "offline");
  ha.setBirthTopic("homeassistant/status", "online");
  ha.setSwitchCommandCallback(onSwitchCommand);

  ha.addSensor(
    "temperature",
    "esp32_salon_temperature",
    "devices/esp32_salon/temperature/state",
    "temperature",
    "°C",
    "measurement",
    "",
    "Temperature"
  );

  ha.addBinarySensor(
    "motion",
    "esp32_salon_motion",
    "devices/esp32_salon/motion/state",
    "motion",
    "ON",
    "OFF",
    "",
    "Motion"
  );

  ha.addSwitch(
    "relay_1",
    "esp32_salon_relay_1",
    "devices/esp32_salon/relay_1/state",
    "devices/esp32_salon/relay_1/set",
    "ON",
    "OFF",
    "Relay 1"
  );

  connectMQTT();
}

unsigned long lastPublish = 0;

void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();
  ha.loop();

  if (millis() - lastPublish > 500) {
    lastPublish = millis();

    temperatureValue += 0.1;
    if (temperatureValue > 25.0) temperatureValue = 21.0;

    motionDetected = !motionDetected;

    ha.publishTopicState(
      "devices/esp32_salon/temperature/state",
      String(temperatureValue, 1),
      true
    );

    ha.publishTopicState(
      "devices/esp32_salon/motion/state",
      motionDetected ? "ON" : "OFF",
      true
    );

    ha.publishTopicState(
      "devices/esp32_salon/relay_1/state",
      relayState ? "ON" : "OFF",
      true
    );
  }
}