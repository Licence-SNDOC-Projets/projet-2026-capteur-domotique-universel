#include "thermostat.h"

Thermostat::Thermostat()
{
    Serial.begin(115200);
    initialiserWifi();
}

Thermostat::~Thermostat() {}

bool Thermostat::configurationConnection() {
    if (communication.getEtatWifi() == DECONNECTER) {
        startWifi();
        return false;
    } else if (communication.getEtatMqtt() == DECONNECTER) {
        initialiserMqtt();
        return false;
    } else {
        return true;
    }
    
}

void Thermostat::nouveauMessage() {

//   communication.receptionDataMQTT();

//   bool flag = communication.getFlag();
//   String topic = communication.getTopic();
//   String message = communication.getMessage();
//   String topicReceptionCanaux = (String)MQTT_TOPIC_RECEPTION_CANAUX;

//   if (flag == NEW_FLAG)
//   {

//     //TODO : action a faire lors de la reception d'un nouveau message mqtt
//     Serial.println(topic);
//     Serial.println(message);

//     communication.setFlag(RESET_FLAG);
//   }

}

void Thermostat::envoieConfig() {

//   bool flagTimerConfig = communication.getFlagTimerConfig();
//   String adressIp = sauvegarde.getIpAdress();
//   String adressMac = sauvegarde.getMacAdress();
//   float puissanceWifi = communication.getPuissanceWifi();

//   if (flagTimerConfig == NEW_FLAG)
//   {
//     communication.setFlagTimerConfig(RESET_FLAG);
//     envoieConfiguration(adressIp, adressMac, puissanceWifi);
//   }
  
}

void Thermostat::startWifi() {
    communication.initialiserWiFi(sauvegarde.getSsidWifi(), sauvegarde.getMdpWifi(), sauvegarde.getNameModuleWifi());
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

        startWifi();

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

    bool etatWifi = communication.getEtatWifi();
    bool etatMqtt = communication.getEtatMqtt();

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

      communication.initialiserMQTT(mqttIp, mqttPort, mqttUser, mqttMdp);
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