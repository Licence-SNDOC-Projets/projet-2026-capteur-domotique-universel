#ifndef HA_EQUIPMENT_TYPES_H
#define HA_EQUIPMENT_TYPES_H

#define DESCOVERY_PREFIX "homeassistant"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

/*
 * Home Assistant entity types
 * - Values preserved from your original defines for backward compatibility.
 * - Added UNKNOWN = 0
 */

typedef enum : uint8_t {
    HA_TYPE_UNKNOWN               = 0,

    HA_TYPE_ALARM_CONTROL_PANEL   = 1,
    HA_TYPE_BINARY_SENSOR         = 2,
    HA_TYPE_BUTTON                = 3,
    HA_TYPE_CAMERA                = 4,
    HA_TYPE_COVER                 = 5,
    HA_TYPE_CLIMATE               = 6,
    HA_TYPE_DEVICE_TRACKER        = 7,
    HA_TYPE_TRIGGER               = 8,
    HA_TYPE_EVENT                 = 9,
    HA_TYPE_FAN                   = 10,
    HA_TYPE_HUMIDIFIER            = 11,
    HA_TYPE_IMAGE                 = 12,
    HA_TYPE_LAWN_MOWER            = 13,
    HA_TYPE_LIGHT                 = 14,
    HA_TYPE_LOCK                  = 15,
    HA_TYPE_NOTIFY                = 16,
    HA_TYPE_NUMBER                = 17,
    HA_TYPE_SCENE                 = 18,
    HA_TYPE_SELECT                = 19,
    HA_TYPE_SENSOR                = 20,
    HA_TYPE_SIREN                 = 21,
    HA_TYPE_SWITCH                = 22,
    HA_TYPE_UPDATE                = 23,
    HA_TYPE_TAG_SCANNER           = 24,
    HA_TYPE_TEXT                  = 25,
    HA_TYPE_VACUUM                = 26,
    HA_TYPE_VALVE                 = 27,
    HA_TYPE_WATER_HEATER          = 28
} ha_equipment_type_t;

/* Canonical Home Assistant strings (as used in MQTT discovery / entity platforms) */
typedef struct {
    ha_equipment_type_t type;
    const char *name;   // canonical string
} ha_type_map_t;

static const ha_type_map_t HA_TYPE_MAP[] = {
    { HA_TYPE_ALARM_CONTROL_PANEL, "alarm_control_panel" },
    { HA_TYPE_BINARY_SENSOR,       "binary_sensor" },
    { HA_TYPE_BUTTON,              "button" },
    { HA_TYPE_CAMERA,              "camera" },
    { HA_TYPE_COVER,               "cover" },
    { HA_TYPE_CLIMATE,             "climate" },
    { HA_TYPE_DEVICE_TRACKER,      "device_tracker" },
    { HA_TYPE_TRIGGER,             "trigger" },
    { HA_TYPE_EVENT,               "event" },
    { HA_TYPE_FAN,                 "fan" },
    { HA_TYPE_HUMIDIFIER,          "humidifier" },
    { HA_TYPE_IMAGE,               "image" },
    { HA_TYPE_LAWN_MOWER,          "lawn_mower" },
    { HA_TYPE_LIGHT,               "light" },
    { HA_TYPE_LOCK,                "lock" },
    { HA_TYPE_NOTIFY,              "notify" },
    { HA_TYPE_NUMBER,              "number" },
    { HA_TYPE_SCENE,               "scene" },
    { HA_TYPE_SELECT,              "select" },
    { HA_TYPE_SENSOR,              "sensor" },
    { HA_TYPE_SIREN,               "siren" },
    { HA_TYPE_SWITCH,              "switch" },
    { HA_TYPE_UPDATE,              "update" },
    { HA_TYPE_TAG_SCANNER,         "tag_scanner" },
    { HA_TYPE_TEXT,                "text" },
    { HA_TYPE_VACUUM,              "vacuum" },
    { HA_TYPE_VALVE,               "valve" },
    { HA_TYPE_WATER_HEATER,        "water_heater" },
};

/* Convert enum -> string */
static inline const char* ha_type_to_string(ha_equipment_type_t type) {
    for (size_t i = 0; i < (sizeof(HA_TYPE_MAP) / sizeof(HA_TYPE_MAP[0])); i++) {
        if (HA_TYPE_MAP[i].type == type) return HA_TYPE_MAP[i].name;
    }
    return "unknown";
}

/* Convert string -> enum (returns HA_TYPE_UNKNOWN if not found) */
static inline ha_equipment_type_t ha_type_from_string(const char *name) {
    if (!name) return HA_TYPE_UNKNOWN;
    for (size_t i = 0; i < (sizeof(HA_TYPE_MAP) / sizeof(HA_TYPE_MAP[0])); i++) {
        if (strcmp(HA_TYPE_MAP[i].name, name) == 0) return HA_TYPE_MAP[i].type;
    }
    return HA_TYPE_UNKNOWN;
}

#ifdef __cplusplus
}
#endif

#endif /* HA_EQUIPMENT_TYPES_H */
