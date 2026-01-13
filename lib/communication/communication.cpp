#include "communication.h"
#include <Arduino.h>

String _mqttTopic;
String _mqttMessage;
String _nomModuleWifi;
int _univers;

String _wifiSsid;
String _wifiMdp;

volatile bool _etatConnexionWifi = DECONNECTER;
volatile bool _etatConnexionMqtt = DECONNECTER;
volatile uint8_t _compteurMqtt = 0;
volatile uint8_t _compteurWifi = 0;
volatile bool _mqttFlagNouveauMessage = RESET_FLAG;
volatile bool _mqttFlagTimerConfig = RESET_FLAG;
hw_timer_t * timerConfig = NULL;

WiFiClient connexionWiFi;
PubSubClient clientMQTT(connexionWiFi);
JsonDocument jsonConfig;

void interruptionWifiConnecter(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
    Serial.println(F("Wifi : Module connecter au reseau"));
}

void interruptionWifiDeconnecter(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
    if (_compteurWifi >= LIMITE_COMPTEUR) ESP.restart();
    Serial.println(F("Wifi : Module non connecter, connexion en cours..."));
    WiFi.begin(_wifiSsid, _wifiMdp);
    _etatConnexionWifi = DECONNECTER;
    _compteurWifi++;
}

void interruptionWifiNouvelleAdressIp(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
    Serial.print(F("Wifi : Nouvelle adress ip : "));
    Serial.println(WiFi.localIP());
    _etatConnexionWifi = CONNECTER;
    _compteurWifi = 0;
}

void interuptionNouveauMessageMQTT(char *mqttTopic, byte *mqttPayload, unsigned int nombreCarathere)
{
    // Pré-alloue la taille du message pour éviter les reallocations
    String message((char*)mqttPayload, nombreCarathere);

    Serial.println(message);

    _mqttFlagNouveauMessage = NEW_FLAG;
    _mqttTopic = mqttTopic;
    _mqttMessage = message;
}

void IRAM_ATTR interuptionEnvoieConfiguration() {
    _mqttFlagTimerConfig = NEW_FLAG;
}

void envoyerMessage(String mqtt_topic, String data)
{
    clientMQTT.publish(mqtt_topic.c_str(), data.c_str());
}

void envoieConfiguration(String adressIp, String adressMac, float puissanceWifi) {

    String topicEnvoieConfig = (String)MQTT_TOPIC_CONFIG_MODULE + "/" + _nomModuleWifi;
    String configModule;

    jsonConfig["ip"] = adressIp;
    jsonConfig["mac"] = adressMac;
    jsonConfig["rssi"] = puissanceWifi;
    serializeJson(jsonConfig, configModule);

    envoyerMessage(topicEnvoieConfig, configModule);
}

void Communication::receptionDataMQTT()
{
    clientMQTT.loop();
}

void Communication::initialiserWiFi(String nomModuleWifi, String ssid, String password)
{

    _nomModuleWifi = nomModuleWifi;
    _wifiSsid = ssid;
    _wifiMdp = password;

    WiFi.mode(WIFI_MODE_STA);

    WiFi.onEvent(interruptionWifiConnecter, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(interruptionWifiNouvelleAdressIp, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(interruptionWifiDeconnecter, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.begin(ssid.c_str(), password.c_str());

}

void Communication::initialiserTimer() {

    timerConfig = timerBegin(NUMERO_TIMER, FREQUENCE_TIMER, true);
    timerAttachInterrupt(timerConfig, &interuptionEnvoieConfiguration, true);
    timerAlarmWrite(timerConfig, TEMPS_ATTENTE, true);
    timerAlarmEnable(timerConfig);

}

void Communication::initialiserMQTT(String mqttBroker, uint16_t mqttPort, String mqttUsername, String mqttPassword)
{
    clientMQTT.setServer(mqttBroker.c_str(), mqttPort);
    clientMQTT.setCallback(interuptionNouveauMessageMQTT);

    Serial.print(F("MQTT : Tentative de connexion au broker..."));
    if (clientMQTT.connect(_nomModuleWifi.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {

        Serial.printf("\r\nMQTT : Le client : %s est connecter au broker\r\n", _nomModuleWifi.c_str());
        this->sinscrireAuxTopic();
        this->initialiserTimer();
        
    }
}

void Communication::sinscrireAuxTopic() {

    String topicLectureCanaux = (String)MQTT_TOPIC_RECEPTION_CANAUX;

    clientMQTT.subscribe(topicLectureCanaux.c_str());

    Serial.print(F("Abonner au topic : "));
    Serial.println(topicLectureCanaux);

}

String Communication::getTopic()
{
    return _mqttTopic;
}

void Communication::setTopic(String mqttTopic)
{
    _mqttTopic = mqttTopic;
}

String Communication::getMessage()
{
    return _mqttMessage;
}

void Communication::setMessage(String mqttMessage)
{
    _mqttMessage = mqttMessage;
}

bool Communication::getFlag()
{
    return _mqttFlagNouveauMessage;
}

void Communication::setFlag(bool mqttFlagNouveauMessage)
{
    _mqttFlagNouveauMessage = mqttFlagNouveauMessage;
}

void Communication::setFlagTimerConfig(bool mqttFlagTimerConfig)
{
    _mqttFlagTimerConfig = mqttFlagTimerConfig;
}

bool Communication::getFlagTimerConfig()
{
    return _mqttFlagTimerConfig;
}

bool Communication::getEtatWifi()
{
    return _etatConnexionWifi;
}

bool Communication::getEtatMqtt()
{
    _etatConnexionMqtt = clientMQTT.connected();
    return _etatConnexionMqtt;
}

float Communication::getPuissanceWifi()
{
  _puissanceWifi = WiFi.RSSI();
  return _puissanceWifi;
}

String Communication::getQualiterWifi()
{
    this->getPuissanceWifi();

    if (_puissanceWifi >= RSSI_TRES_BON)
    {
        return "Tres bon";
    } else if(_puissanceWifi >= RSSI_ASSEZ_BON) 
    {
        return "Assez bon";
    } else if(_puissanceWifi >= RSSI_PASSABLE) 
    {
        return "Passable";
    } else if(_puissanceWifi >= RSSI_PAS_BON) 
    {
        return "Pas bon";
    } else
    {
        return "Mediocre";
    }
  
}