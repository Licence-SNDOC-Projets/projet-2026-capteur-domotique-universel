#include <Arduino.h>
#include "configuration.h"
#include "communication.h"

#define TEMPS_ATTENTE_MQTT 2000

Configuration sauvegarde;
Communication transmission;

void initialiserWifi() {

  String wifiSsid = sauvegarde.getSsidWifi();
  String wifiMdp = sauvegarde.getMdpWifi();
  String nomModuleWifi = sauvegarde.getNameModuleWifi();

  Serial.println(F("Connexion au WiFi : "));
  Serial.println("wifiSsid : " + wifiSsid);
  Serial.println("wifiMdp : " + wifiMdp);
  Serial.println("nomModuleWifi : " + nomModuleWifi);
  transmission.initialiserWiFi(nomModuleWifi, wifiSsid, wifiMdp);

}

void initialiserMqtt() {

  static long currentTime = 0;
  static unsigned long previousTime = 0;
  currentTime = millis();

  if((currentTime - previousTime) >= TEMPS_ATTENTE_MQTT) {

    bool etatWifi = transmission.getEtatWifi();
    bool etatMqtt = transmission.getEtatMqtt();

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

      transmission.initialiserMQTT(mqttIp, mqttPort, mqttUser, mqttMdp);
    }


    previousTime = currentTime;

  }

}

void setup()
{

  Serial.begin(115200);
  sauvegarde.initialiserMemoire();

  if (sauvegarde.configurationSauvegarder() == true)
  {

    Serial.println(F("Recuperation des donners sauvegarder :"));

    initialiserWifi();

  } else {
    Serial.println(F("pas de valeur sauvegarder !"));
    sauvegarde.creationPointAcces();
    delay(2000);
  }

  sauvegarde.creationServeurWeb();
}

void reinitilisationModule() {

  bool flagResetConfig = sauvegarde.getFlagResetConfig();

  if (flagResetConfig == NEW_FLAG)
  {
    flagResetConfig = RESET_FLAG;
    resetConfiguration();
  }
  
}

void nouveauMessage() {

  transmission.receptionDataMQTT();

  bool flag = transmission.getFlag();
  String topic = transmission.getTopic();
  String message = transmission.getMessage();
  String topicReceptionCanaux = (String)MQTT_TOPIC_RECEPTION_CANAUX;

  if (flag == NEW_FLAG)
  {

    //TODO : action a faire lors de la reception d'un nouveau message mqtt
    Serial.println(topic);
    Serial.println(message);

    transmission.setFlag(RESET_FLAG);
  }

}

void envoieConfig() {

  bool flagTimerConfig = transmission.getFlagTimerConfig();
  String adressIp = sauvegarde.getIpAdress();
  String adressMac = sauvegarde.getMacAdress();
  float puissanceWifi = transmission.getPuissanceWifi();

  if (flagTimerConfig == NEW_FLAG)
  {
    transmission.setFlagTimerConfig(RESET_FLAG);
    envoieConfiguration(adressIp, adressMac, puissanceWifi);
  }
  
}

void loop()
{
//   reinitilisationModule();

  bool etatWifi = transmission.getEtatWifi();
  bool etatMqtt = transmission.getEtatMqtt();

  if (etatWifi == CONNECTER && etatMqtt == CONNECTER)
  {
    nouveauMessage();
    envoieConfig();
  } else {
    initialiserMqtt();
  }
}