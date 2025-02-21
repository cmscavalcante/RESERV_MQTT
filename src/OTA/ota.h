#ifndef OTA_H
#define OTA_H

#include "MQTT\mqttSys.h"

#include <ArduinoOTA.h>

class mqtt;  // Forward declaration

extern std::atomic<bool> flowMeasurementEnable; // extern declaration// No implementation details here!

class otaHandle {
public:
    static void otaInit(mqtt& Mqtt); // Interface declaration

private:
   
};

#endif // OTA_H