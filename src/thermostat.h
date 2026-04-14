#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "configuration.h"
#include "communication.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "HAMqttDeviceDiscovery.h"

#define TEMPS_ATTENTE_MQTT 2000

WiFiClient connexionWiFi;
PubSubClient clientMQTT(connexionWiFi);
Configuration sauvegarde;
Communication communication(clientMQTT);

class Thermostat
{
private:

    void initialiserMqtt();
    void initialiserWifi();
    void reinitilisationModule();
    void startWifi();

public:
    Thermostat();
    ~Thermostat();

    bool configurationConnection();
};


#endif