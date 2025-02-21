#ifndef WIFI_CONFIGS_H
#define WIFI_CONFIGS_H
#include <Arduino.h>
#include "iot_configs.h"

// Wifi Parameters for two SSIDs
#define WIFI_SSID_1 "CCAVALCA"
#define WIFI_PASSWORD_1 "12345678"
#define WIFI_SSID_2 "CEARA_1"
#define WIFI_PASSWORD_2 "1020304050"
#define WIFI_LOCAL_IP {10, 0, 2, 203}
#define WIFI_GATEWAY {10, 0, 2, 1}
#define WIFI_SUBNET {255, 255, 255, 0}
#define WIFI_DNS_PRI {8, 8, 8, 8}
#define WIFI_DNS_SEC {8, 8, 4, 4}
#define INTERNET_TIME_OUT_MS {5000}
const char* hostName = HOSTNAME;

#endif //WIFI_CONFIGS_H