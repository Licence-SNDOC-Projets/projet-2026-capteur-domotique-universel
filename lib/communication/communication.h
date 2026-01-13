#ifndef _COMMUNICATION_H__
#define __COMMUNICATION_H__

#define MQTT_MAX_PACKET_SIZE 1024

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "esp_timer.h"
#include <ArduinoJson.h>

#define WIFI_MDP ""
#define WIFI_CHANNEL 0

#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_MDP ""

#define MQTT_TOPIC_GENERALE "sndoc/"
#define MQTT_TOPIC_CONFIG_MODULE  MQTT_TOPIC_GENERALE + "config"
#define MQTT_TOPIC_RECEPTION_CANAUX  ""

#define RESET_FLAG false
#define NEW_FLAG true
#define DECONNECTER false
#define CONNECTER true
#define LIMITE_COMPTEUR 10

// Timer d'envoie de la configuration (15s)
#define NUMERO_TIMER 3
#define FREQUENCE_TIMER 80
#define TEMPS_ATTENTE 15000000

#define RSSI_TRES_BON -55
#define RSSI_ASSEZ_BON -67
#define RSSI_PASSABLE -70
#define RSSI_PAS_BON -80

class Communication
{

public:
    void initialiserWiFi(String nomModuleWifi, String ssid, String password = WIFI_MDP);
    void initialiserMQTT(String mqttBroker, uint16_t mqttPort = MQTT_PORT, String mqttUsername = MQTT_USER, String mqttPassword = MQTT_MDP);
    void receptionDataMQTT();

    String getTopic();
    void setTopic(String mqttTopic);
    String getMessage();
    void setMessage(String mqttMessage);
    bool getFlag();
    void setFlag(bool mqttFlag);
    void setFlagTimerConfig(bool mqttFlagTimerConfig);
    bool getFlagTimerConfig();
    bool getEtatWifi();
    bool getEtatMqtt();
    float getPuissanceWifi();
    String getQualiterWifi();

private:
    float _puissanceWifi;
    void sinscrireAuxTopic();
    void initialiserTimer();

};

void envoieConfiguration(String adressIp, String adressMac, float puissanceWifi);
void envoyerMessage(String mqtt_topic, String data);

#endif