#include <Arduino.h>
#include <atomic>
#include "OTA.h"

#include "interruptControl.h"
 // All implementation-specific includes here

extern bool mqttHalter;

void otaHandle::otaInit(mqtt& Mqtt) { // Implementation
    ArduinoOTA.onStart([&Mqtt]() {
        String type;
        detachVazaoInterrupt();
        mqttHalter=true;
        flowMeasurementEnable = false;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        // *** Disable flow measurement during OTA ***
        
        char topic[10];
        char msg[100];
        snprintf(topic, sizeof(topic), "OTA");
        snprintf (msg, sizeof(msg), "Start updating %s. Flow measurement disabled", type.c_str());
        Mqtt.mqttSendMessage(topic, msg);
        Mqtt.getClient()->disconnect();
    });
    ArduinoOTA.onEnd([&Mqtt]() {
        Serial.println("\nEnd");
        char topic[10];
        char msg[100];
        snprintf(topic, sizeof(topic), "OTA");
        snprintf(msg, sizeof(msg), "End update. System will restart.");
        Mqtt.mqttSendMessage(topic, msg);
        Mqtt.getClient()->disconnect();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([&Mqtt](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        String errorStr;
        if (error == OTA_AUTH_ERROR) errorStr="Auth Failed";
        else if (error == OTA_BEGIN_ERROR) errorStr="Begin Failed";
        else if (error == OTA_CONNECT_ERROR) errorStr="Connect Failed";
        else if (error == OTA_RECEIVE_ERROR) errorStr="Receive Failed";
        else if (error == OTA_END_ERROR) errorStr="End Failed";
        flowMeasurementEnable = true;
        char topic[10];
        char msg[100];
        snprintf(topic, sizeof(topic), "OTA");
        snprintf(msg, sizeof(msg), "Update error[%u] - %s", error, errorStr.c_str());
        Mqtt.mqttSendMessage(topic, msg);
        attachVazaoInterrupt();
        mqttHalter=false;
    });
    ArduinoOTA.begin();
}