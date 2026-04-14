#ifndef _COMMUNICATION_H__
#define __COMMUNICATION_H__

#define MQTT_MAX_PACKET_SIZE 1024

#include <Arduino.h>
#include "esp_timer.h"
#include <ArduinoJson.h>

#define WIFI_MDP ""
#define WIFI_CHANNEL 0

#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_MDP ""

#define RESET_FLAG false
#define NEW_FLAG true
#define DECONNECTER false
#define CONNECTER true
#define LIMITE_COMPTEUR 10

#define RSSI_TRES_BON -55
#define RSSI_ASSEZ_BON -67
#define RSSI_PASSABLE -70
#define RSSI_PAS_BON -80

class ClientMQTT;

class Communication
{

public:

    Communication(ClientMQTT& clientMqtt);
    ~Communication();

    void initialiserWiFi(String nomModuleWifi, String ssid, String password = WIFI_MDP);
    void initialiserMQTT(String mqttBroker, uint16_t mqttPort = MQTT_PORT, String mqttUsername = MQTT_USER, String mqttPassword = MQTT_MDP);
    void receptionDataMQTT();

    String getTopic();
    void setTopic(String mqttTopic);
    String getMessage();
    void setMessage(String mqttMessage);
    bool getFlag();
    void setFlag(bool mqttFlag);
    bool getEtatWifi();
    bool getEtatMqtt();
    float getPuissanceWifi();
    String getQualiterWifi();

private:
    ClientMQTT* _clientMqtt;
    float _puissanceWifi;
    // void sinscrireAuxTopic();

    void setClientMqtt(ClientMQTT& clientMqtt);

};

void envoyerMessage(String mqtt_topic, String data);

#endif