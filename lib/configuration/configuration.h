#ifndef _CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <SPIFFS.h>

#define PIN_BOUTON_RESET 19
#define TEMPS_RESET_BOUTON 5000
#define TEMPS_REBONS 210

#define POINT_ACCES_SSID "Sndoc-"
#define POINT_ACCES_MDP ""
#define PORT_SERVEUR_WEB 80

#define MQTT_PORT 1883

#define CLE_MEMOIRE_WIFI_SSID "wifi_ssid"
#define CLE_MEMOIRE_WIFI_MDP "wifi_mdp"

#define CLE_MEMOIRE_MQTT_IP "mqtt_ip"
#define CLE_MEMOIRE_MQTT_PORT "mqtt_port"
#define CLE_MEMOIRE_MQTT_USER "mqtt_user"
#define CLE_MEMOIRE_MQTT_MDP "mqtt_mdp"

#define CLE_MEMOIRE_UNIVERS "univers"

#define RESET_FLAG false
#define NEW_FLAG true

class Configuration
{
    public:
        void initialiserMemoire();
        void initialiserBoutonReset();
        void creationServeurWeb();
        void creationPointAcces();
        
        bool configurationSauvegarder();
        String getSsidWifi();
        String getMdpWifi();
        String getIpMqtt();
        int getPortMqtt();
        String getUserMqtt();
        String getMdpMqtt();
        int getUnivers();


        String getNameModuleWifi();
        String getIpAdress();
        String getMacAdress();

        bool getFlagResetConfig();

    private:
        void lireMemoire();

        String _ssidWifi;
        String _mdpWifi;
        String _ipMqtt;
        int _portMqtt;
        String _userMqtt;
        String _mdpMqtt;
        int _univers;
        String _nomModuleWifi;
};

void resetConfiguration();

#endif