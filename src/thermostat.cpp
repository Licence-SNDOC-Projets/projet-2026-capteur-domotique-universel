#include "thermostat.h"
#include <PubSubClient.h>

Configuration sauvegarde;
WiFiClient connexionWiFi;
PubSubClient clientMQTT(connexionWiFi);
HAMqttDeviceDiscovery ha(clientMQTT);

HAMqttDeviceDiscovery::DeviceInfo device = {
    "esp32_thermostat",     // ids
    "Thermostat ESP32",     // name
    "MonFabricant",         // mf
    "ESP32 Thermostat",     // mdl
    "THERMO_V1",            // mdl_id
    "1.0.0",                // sw
    "123456",               // sn
    "REV_A",                // hw
    "http://192.168.1.10"   // cu
};

HAMqttDeviceDiscovery::OriginInfo origin = {
    "MaLibHA",
    "1.0.0",
    "https://monsite.fr"
};

void onSwitchCommand(const String& id, const String& payload)
{
    if (id == "chauffage") {
        if (payload == "ON") {
            Serial.println("Chauffage ON");
        } else {
            Serial.println("Chauffage OFF");
        }
    }
}

Thermostat::Thermostat() {
    Serial.begin(115200);
}

Thermostat::~Thermostat() {}

bool Thermostat::connection() {

    if (ha.getEtatWifi() == CONNECTER && ha.getEtatMqtt() == DECONNECTER) {
        initialiserMqtt();
        return false;
    } else if (ha.getEtatWifi() == DECONNECTER || ha.getEtatMqtt() == DECONNECTER) {
        return false;
    } else {
        return true;
    }
    
}

void Thermostat::configurationEquipement() {

    // Home Assistant
    ha.begin(device, origin);

    // disponibilité (important pour HA)
    ha.setAvailability("maison/thermostat/status");

    // capteur température
    ha.addSensor(
        "temperature",
        "esp32_temperature",
        "maison/thermostat/temperature",
        "temperature",
        "°C"
    );

    // switch chauffage
    ha.addSwitch(
        "chauffage",
        "esp32_chauffage",
        "maison/thermostat/chauffage/state",
        "maison/thermostat/chauffage/set"
    );

    // envoyer la config à Home Assistant
    ha.publishDiscovery();

    // dire que l’appareil est en ligne
    ha.publishAvailability(true);

    ha.setSwitchCommandCallback(onSwitchCommand);
}

void Thermostat::procedure() {
    // MQTT loop (via classe mère)
    ha.loop();

    // gestion des messages MQTT entrants
    ha.handleMessage(ha.getTopic(), ha.getMessage());

    // Exemple : envoyer température toutes les 5 sec
    static unsigned long last = 0;
    if (millis() - last > 5000) {
        last = millis();

        float temperature = 22.5;

        ha.publishComponentState("temperature", String(temperature));
    }
}

void Thermostat::initialiserWifi() {

    sauvegarde.initialiserMemoire();

    if (sauvegarde.configurationSauvegarder() == true)
    {

        Serial.println(F("Recuperation des donners sauvegarder :"));

        Serial.println(F("Connexion au WiFi : "));
        Serial.println("wifiSsid : " + sauvegarde.getSsidWifi());
        Serial.println("wifiMdp : " + sauvegarde.getMdpWifi());
        Serial.println("nomModuleWifi : " + sauvegarde.getNameModuleWifi());

        ha.initialiserWiFi(sauvegarde.getNameModuleWifi(), sauvegarde.getSsidWifi(), sauvegarde.getMdpWifi());

    } else {
        Serial.println(F("pas de valeur sauvegarder !"));
        sauvegarde.creationPointAcces();
        delay(2000);
    }

    sauvegarde.creationServeurWeb();

}

void Thermostat::initialiserMqtt() {

  static long currentTime = 0;
  static unsigned long previousTime = 0;
  currentTime = millis();

  if((currentTime - previousTime) >= TEMPS_ATTENTE_MQTT) {

    bool etatWifi = ha.getEtatWifi();
    bool etatMqtt = ha.getEtatMqtt();

    if(etatWifi == CONNECTER && etatMqtt == DECONNECTER) 
    {

      String mqttIp = sauvegarde.getIpMqtt();
      int mqttPort = sauvegarde.getPortMqtt();
      String mqttUser = sauvegarde.getUserMqtt();
      String mqttMdp = sauvegarde.getMdpMqtt();

      Serial.println(F("Connexion au MQTT : "));
      Serial.println("mqttIp : " + mqttIp);
      Serial.println("mqttPort : " + (String)mqttPort);
      Serial.println("mqttUser : " + mqttUser);
      Serial.println("mqttMdp : " + mqttMdp);

      ha.initialiserMQTT(mqttIp, mqttPort, mqttUser, mqttMdp);
    }


    previousTime = currentTime;

  }

}

void Thermostat::reinitilisationModule() {

  bool flagResetConfig = sauvegarde.getFlagResetConfig();

  if (flagResetConfig == NEW_FLAG)
  {
    flagResetConfig = RESET_FLAG;
    resetConfiguration();
  }
  
}