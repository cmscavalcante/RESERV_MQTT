#ifndef MQTTSYS_H
#define MQTTSYS_H

#include <Arduino.h>
#include <Esp.h>
#include <PubSubClient.h>
#include "iot_configs.h"
#include "medReserv\medReserv.h"

extern std::atomic<bool> flowMeasurementEnable; // extern declaration

class mqtt
{
    public:
        mqtt(medReserv& medReservRef); // Constructor now takes a reference
        //mqtt(void);
        void mqttInit(void);
        //Getters
        int getTelemetryFreq() const;
        bool getPumpManualSet() const;
        //Setters
        void setPumpManualSet(bool set);
        // bool getFlowReadEnable() const;
        void mqttSendTelWater(bool water);
        void mqttSendTelFull(bool full);
        void mqttSendTelPump(bool pump);
        void mqttSendTelFlow(float flow);
        void mqttSendTelVolReserv(float volReserv);
        void mqttSendTelDist(float dist);
        void mqttSendTelDistError(int erro);
        void mqttSendMessage(const char* topic, const char* message);
        void mqttSendReport(void);
        void mqttLoop(void);
        PubSubClient* getClient();
        

    private:
        medReserv& measurementReserv; // Reference to medReserv object
        bool mqttCallbackCompareTopic(const char* topic, const char* expectedTopic);
        char* buildMessage(const char* message);
        char* buildTopic(const char* subtopic);
        void mqttReconnect(void);
        static void mqttCallbackStatic(char* topic, byte* payload, unsigned int length); // Static bridge
        void mqttCallback(char* topic, byte* payload, unsigned int length); // Instance callback
        static mqtt* mqttInstance; //Static pointer to the instance callback
        PubSubClient * client; // MQTT Client

        //bool flowReadEnable = false;
        // MQTT Topics
        const char* rootTopic = HOSTNAME;
        const char* allTopicsReserv =  "#"; // Subscribe to all topics under root
        const char* allTopicsConsts = "constantes/#";
        const char* isConnectedTopic =  "isConnected";
        const char* volResTopic = "VolData";
        // static const char* volInTopic = "VolInData";
        const char* waterTopic = "water";
        const char* flowTopic = "flow";
        const char* flowReadEnableTopic = "flowReadEnable";
        const char* fullTopic = "full";
        const char* pumpTopic = "pump";
        const char* distCompTopic = "constantes/distComp"; // This one is already ok
        const char* distTopic = "distancia";
        const char* report = "statusReport";
        const char* restart = "restart";
        const char* lastWillTopic = "will";
        const char* distErrorTopic = "distError";
        // MQTT Configs
        static const int mqttPort = 8883;
        static const int mqttTelemertyFreq = TELEMETRY_FREQUENCY_MILLISECS;
        const char* mqttUsername = MQTT_USERNAME;
        const char* mqttPassword = MQTT_PASSWORD;
        const char* mqttServer = MQTT_SERVER;
        
        const String mqttClientId = "ESP-" + String(ESP.getChipId());

};


#endif // MQTTSYS_H