#ifndef HA_MQTT_TYPES_H
#define HA_MQTT_TYPES_H

#include <Arduino.h>

enum class HAComponentType
{
    SENSOR,
    BINARY_SENSOR,
    SWITCH,
    BUTTON,
    NUMBER,
    SELECT,
    TEXT,
    LIGHT,
    FAN,
    CLIMATE,
    COVER,
    LOCK,
    VALVE,
    SIREN,
    ALARM_CONTROL_PANEL,
    CAMERA,
    IMAGE,
    UPDATE,
    VACUUM,
    HUMIDIFIER,
    WATER_HEATER,
    SCENE,
    EVENT,
    DEVICE_AUTOMATION
};

// Conversion enum → string Home Assistant
inline const char* HAComponentTypeToString(HAComponentType type)
{
    switch (type)
    {
        case HAComponentType::SENSOR: return "sensor";
        case HAComponentType::BINARY_SENSOR: return "binary_sensor";
        case HAComponentType::SWITCH: return "switch";
        case HAComponentType::BUTTON: return "button";
        case HAComponentType::NUMBER: return "number";
        case HAComponentType::SELECT: return "select";
        case HAComponentType::TEXT: return "text";
        case HAComponentType::LIGHT: return "light";
        case HAComponentType::FAN: return "fan";
        case HAComponentType::CLIMATE: return "climate";
        case HAComponentType::COVER: return "cover";
        case HAComponentType::LOCK: return "lock";
        case HAComponentType::VALVE: return "valve";
        case HAComponentType::SIREN: return "siren";
        case HAComponentType::ALARM_CONTROL_PANEL: return "alarm_control_panel";
        case HAComponentType::CAMERA: return "camera";
        case HAComponentType::IMAGE: return "image";
        case HAComponentType::UPDATE: return "update";
        case HAComponentType::VACUUM: return "vacuum";
        case HAComponentType::HUMIDIFIER: return "humidifier";
        case HAComponentType::WATER_HEATER: return "water_heater";
        case HAComponentType::SCENE: return "scene";
        case HAComponentType::EVENT: return "event";
        case HAComponentType::DEVICE_AUTOMATION: return "device_automation";
        default: return "sensor";
    }
}

enum class HADeviceClass
{
    TEMPERATURE,
    HUMIDITY,
    MOTION,
    DOOR,
    POWER,
    VOLTAGE,
    CURRENT,
    BATTERY
};

#endif