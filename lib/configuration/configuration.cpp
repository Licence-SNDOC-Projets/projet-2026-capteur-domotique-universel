#include "configuration.h"
#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

volatile bool _flagResetModule = RESET_FLAG;

AsyncWebServer serverWeb(PORT_SERVEUR_WEB);
Preferences memoire;

void IRAM_ATTR appuisBoutonReset() {
  static unsigned long tempsCourant  = 0;
  static unsigned long nouveauTemps = 0;

  tempsCourant = millis();
  if((tempsCourant - nouveauTemps) > TEMPS_REBONS) {
    nouveauTemps = tempsCourant;

    _flagResetModule = NEW_FLAG;

  }
}

void interruptionWifiDemmaragePointAcces(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println(F("Wifi : Demarrage point d'acces"));
}

void interruptionWifiNouveauClient(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println(F("Wifi : Nouvelle connexion"));
}

void Configuration::initialiserBoutonReset() {
  pinMode(PIN_BOUTON_RESET, INPUT_PULLUP);
	attachInterrupt(PIN_BOUTON_RESET, appuisBoutonReset, FALLING);
}

void Configuration::initialiserMemoire() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  this->initialiserBoutonReset();

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    Serial.println("NVS corrompu, effacement...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }

  if (err != ESP_OK) {
    Serial.printf("Erreur d'initialisation NVS : %s\n", esp_err_to_name(err));
    return;
  } else {
    Serial.println("NVS initialisé avec succès.");
  }

  if(!SPIFFS.begin(true))
  {
    Serial.println(F("Erreur SPIFFS..."));
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

  this->lireMemoire();
}

void Configuration::lireMemoire() {

  if (memoire.begin("config", false))
  {
    Serial.println("Préférences ouvertes.");

    _ssidWifi = memoire.getString(CLE_MEMOIRE_WIFI_SSID, "");
    _mdpWifi = memoire.getString(CLE_MEMOIRE_WIFI_MDP, "");

    _ipMqtt = memoire.getString(CLE_MEMOIRE_MQTT_IP, "");
    _portMqtt = memoire.getInt(CLE_MEMOIRE_MQTT_PORT, MQTT_PORT);
    _userMqtt = memoire.getString(CLE_MEMOIRE_MQTT_USER, "");
    _mdpMqtt = memoire.getString(CLE_MEMOIRE_MQTT_MDP, "");

    _univers = memoire.getInt(CLE_MEMOIRE_UNIVERS, 0);
    memoire.end();
  } else {
    Serial.println("Erreur à l'ouverture des préférences !");
  }

  if (_portMqtt == 0) _portMqtt = 1883;
  
}

void Configuration::creationServeurWeb() {

  serverWeb.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  serverWeb.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/w3.css", "text/css");
  });

  serverWeb.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  serverWeb.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){

    String ssid = request->hasParam("wifi_ssid") ? request->getParam("wifi_ssid")->value() : "";
    String wifiMdp = request->hasParam("wifi_mdp") ? request->getParam("wifi_mdp")->value() : "";
    String mqttIp = request->hasParam("mqtt_ip") ? request->getParam("mqtt_ip")->value() : "";
    String mqttPort = request->hasParam("mqtt_port") ? request->getParam("mqtt_port")->value() : (String)MQTT_PORT;
    String mqttUser = request->hasParam("mqtt_user") ? request->getParam("mqtt_user")->value() : "";
    String mqttMdp = request->hasParam("mqtt_mdp") ? request->getParam("mqtt_mdp")->value() : "";
    String univers = request->hasParam("univers") ? request->getParam("univers")->value() : "";

    if (mqttPort == 0) mqttPort = 1883;

    memoire.begin("config", false);
    memoire.putString(CLE_MEMOIRE_WIFI_SSID, ssid);
    memoire.putString(CLE_MEMOIRE_WIFI_MDP, wifiMdp);

    memoire.putString(CLE_MEMOIRE_MQTT_IP, mqttIp);
    memoire.putInt(CLE_MEMOIRE_MQTT_PORT, mqttPort.toInt());
    memoire.putString(CLE_MEMOIRE_MQTT_USER, mqttUser);
    memoire.putString(CLE_MEMOIRE_MQTT_MDP, mqttMdp);

    memoire.putInt(CLE_MEMOIRE_UNIVERS, univers.toInt());
    memoire.end();

    Serial.println(F("Nouvelle configuration recue !"));

    Serial.print("ssid : ");
    Serial.println(ssid);
    Serial.print("mdp : ");
    Serial.println(wifiMdp);

    Serial.print("mqttIp : ");
    Serial.println(mqttIp);
    Serial.print("mqttPort : ");
    Serial.println(mqttPort);
    Serial.print("mqttUser : ");
    Serial.println(mqttUser);
    Serial.print("mqttMdp : ");
    Serial.println(mqttMdp);

    Serial.print("univers : ");
    Serial.println(univers);

    request->send(200, "text/plain", "Configuration sauvegardée. Redémarrage...");
    Serial.print(F("Configuration sauvegardée. Redémarrage..."));
    delay(2000);
    ESP.restart();
  });

  serverWeb.begin();

}

void Configuration::creationPointAcces() {

  String nomModuleWifi = this->getNameModuleWifi();

  WiFi.mode(WIFI_MODE_AP);
  WiFi.onEvent(interruptionWifiDemmaragePointAcces, ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(interruptionWifiNouveauClient, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.softAP(nomModuleWifi, POINT_ACCES_MDP);
}

bool Configuration::configurationSauvegarder() {

  if (_ssidWifi != "" && _ipMqtt != "" && _univers != 0)
  {
    return true;
  }

  return false;

}

void resetConfiguration() {
  memoire.begin("config", false);
  memoire.remove(CLE_MEMOIRE_WIFI_SSID);
  memoire.remove(CLE_MEMOIRE_WIFI_MDP);

  memoire.remove(CLE_MEMOIRE_MQTT_IP);
  memoire.remove(CLE_MEMOIRE_MQTT_PORT);
  memoire.remove(CLE_MEMOIRE_MQTT_USER);
  memoire.remove(CLE_MEMOIRE_MQTT_MDP);

  memoire.remove(CLE_MEMOIRE_UNIVERS);
  memoire.end();

  delay(1000);

  ESP.restart();
}

String Configuration::getNameModuleWifi()
{
  String adressMac = this->getMacAdress();
  String dernierNumeroAdressMac = adressMac.substring(adressMac.length() - 5);
  dernierNumeroAdressMac.replace(":", "");
  _nomModuleWifi = POINT_ACCES_SSID + dernierNumeroAdressMac;
  return _nomModuleWifi;
}

String Configuration::getMacAdress()
{
  String adressMac = WiFi.macAddress();
  return adressMac;
}

String Configuration::getIpAdress()
{
  String adressIp = WiFi.localIP().toString();
  return adressIp;
}

String Configuration::getSsidWifi() {
  return _ssidWifi;
}

String Configuration::getMdpWifi() {
  return _mdpWifi;
}

String Configuration::getIpMqtt() {
  return _ipMqtt;
}

int Configuration::getPortMqtt() {
  return _portMqtt;
}

String Configuration::getUserMqtt() {
  return _userMqtt;
}

String Configuration::getMdpMqtt() {
  return _mdpMqtt;
}

int Configuration::getUnivers() {
  return _univers;
}

bool Configuration::getFlagResetConfig() {
  return _flagResetModule;
}