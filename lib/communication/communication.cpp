#include "communication.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

JsonDocument jsonConfig;

String _mqttTopic;
String _mqttMessage;
String _nomModuleWifi;

String _wifiSsid;
String _wifiMdp;

volatile bool _etatConnexionWifi = DECONNECTER;
volatile bool _etatConnexionMqtt = DECONNECTER;
volatile uint8_t _compteurMqtt = 0;
volatile uint8_t _compteurWifi = 0;
volatile bool _mqttFlagNouveauMessage = RESET_FLAG;

void interruptionWifiConnecter(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
    Serial.println(F("Wifi : Module connecter au reseau"));
}

void interruptionWifiDeconnecter(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
    _etatConnexionWifi = DECONNECTER;

    Serial.print(F("Wifi : deconnecte, raison = "));
    Serial.println(wifi_info.wifi_sta_disconnected.reason);

    if (_compteurWifi >= LIMITE_COMPTEUR) {
        Serial.println(F("Wifi : trop d'echecs, restart"));
        ESP.restart();
    }

    _compteurWifi++;
    WiFi.begin(_wifiSsid.c_str(), _wifiMdp.c_str());
}

void interruptionWifiNouvelleAdressIp(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
    Serial.print(F("Wifi : Nouvelle adress ip : "));
    Serial.println(WiFi.localIP());
    _etatConnexionWifi = CONNECTER;
    _compteurWifi = 0;
}

void interuptionNouveauMessageMQTT(char *mqttTopic, byte *mqttPayload, unsigned int nombreCarathere)
{
    String message((char*)mqttPayload, nombreCarathere);

    Serial.println(message);

    _mqttFlagNouveauMessage = NEW_FLAG;
    _mqttTopic = mqttTopic;
    _mqttMessage = message;
}

Communication::Communication(PubSubClient& clientMqtt)
    : _clientMqtt(&clientMqtt) {}

Communication::~Communication() {}

void Communication::receptionDataMQTT()
{
    if (_clientMqtt != nullptr) {
        _clientMqtt->loop();
    }
}

void Communication::initialiserWiFi(String nomModuleWifi, String ssid, String password)
{
    _nomModuleWifi = nomModuleWifi;
    _wifiSsid = ssid;
    _wifiMdp = password;

    WiFi.onEvent(interruptionWifiConnecter, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(interruptionWifiNouvelleAdressIp, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(interruptionWifiDeconnecter, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // utile sur certains réseaux
    delay(100);

    Serial.print(F("wifiSsid : "));
    Serial.println(_wifiSsid);
    Serial.print(F("nomModuleWifi : "));
    Serial.println(_nomModuleWifi);

    WiFi.begin(_wifiSsid.c_str(), _wifiMdp.c_str());

Serial.print("Tentative connexion a : ");
Serial.println(_wifiSsid);

wl_status_t status = WiFi.status();
Serial.print("Status initial : ");
Serial.println(status);
}

void Communication::initialiserMQTT(String mqttBroker, uint16_t mqttPort, String mqttUsername, String mqttPassword)
{
    if (_clientMqtt == nullptr) return;

    _clientMqtt->setServer(mqttBroker.c_str(), mqttPort);
    _clientMqtt->setCallback(interuptionNouveauMessageMQTT);

    Serial.print(F("MQTT : Tentative de connexion au broker..."));
    if (_clientMqtt->connect(_nomModuleWifi.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) {
        Serial.printf("\r\nMQTT : Le client : %s est connecter au broker\r\n", _nomModuleWifi.c_str());
    }
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

bool Communication::getEtatWifi()
{
    return _etatConnexionWifi;
}

bool Communication::getEtatMqtt()
{
    if (_clientMqtt == nullptr) return false;
    _etatConnexionMqtt = _clientMqtt->connected();
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

    if (_puissanceWifi >= RSSI_TRES_BON) {
        return "Tres bon";
    } else if (_puissanceWifi >= RSSI_ASSEZ_BON) {
        return "Assez bon";
    } else if (_puissanceWifi >= RSSI_PASSABLE) {
        return "Passable";
    } else if (_puissanceWifi >= RSSI_PAS_BON) {
        return "Pas bon";
    } else {
        return "Mediocre";
    }
}