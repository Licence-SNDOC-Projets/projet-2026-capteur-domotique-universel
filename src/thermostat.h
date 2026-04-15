#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "configuration.h"
#include "communication.h"
#include "HAMqttDeviceDiscovery.h"

#define TEMPS_ATTENTE_MQTT 2000

class Thermostat
{
public:
    Thermostat();
    ~Thermostat();

    void initialiserMqtt();
    void initialiserWifi();
    void reinitilisationModule();

    bool connection();
    void configurationEquipement();
    void procedure();
};


#endif