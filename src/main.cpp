#include <Arduino.h>
#include "thermostat.h"

Thermostat thermostat;

void setup() {
  thermostat.initialiserWifi();
  thermostat.configurationEquipement();
}


void loop() {
  if (thermostat.connection())
  {
    thermostat.procedure();
  }
}