#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H
#include <Arduino.h>
#include <Esp.h>

// Wifi
#define IOT_CONFIG_WIFI_SSID_1 "CCAVALCA"
#define IOT_CONFIG_WIFI_PASSWORD_1 "12345678"
#define IOT_CONFIG_WIFI_SSID_2 "CEARA_1"
#define IOT_CONFIG_WIFI_PASSWORD_2 "1020304050"
#define IOT_CONFIG_TIME_OUT_MS {10000}

// MQTT

#define HOSTNAME "reservatorio"
#define MQTT_SERVER "9f069cb597904de3bd31199f7ea60f27.s1.eu.hivemq.cloud"
#define MQTT_USERNAME "Reserv"
#define MQTT_PASSWORD "Ccava1ca"

// Publish telemetry every 30 seconds
#define TELEMETRY_FREQUENCY_MILLISECS 30000

#endif // IOT_CONFIGS_H