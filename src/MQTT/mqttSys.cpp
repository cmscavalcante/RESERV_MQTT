#include <Arduino.h>
#include <string>
#include <atomic>

#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <EEPROM.h>

#include "mqttSys.h"
#include "iot_configs.h"
#include "SysIni\sysIni.h"
#include "Time\hora.h"

#define MQTT_PACKET_SIZE 1024
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;

mqtt* mqtt::mqttInstance = nullptr;

BearSSL::CertStore certStore;
WiFiClientSecure espclient;
PubSubClient* client;

Hora quando;

extern String dayTime, riseStr, setStr, dataStr, horaStr;
extern String lastTelemetry;
extern int distCompensationAddr;
extern float distCompensation;
extern bool agua, cheio, bomba;

bool PUMP_MANUAL = false;

mqtt::mqtt(medReserv& medReservRef) : measurementReserv(medReservRef) {} //

//Getter for the client
PubSubClient* mqtt::getClient() { return client; }

// Helper function to concatenate rootTopic and subtopic
char* mqtt::buildTopic(const char* subtopic) {
    if (rootTopic == nullptr || subtopic == nullptr) {
        return nullptr; // Handle null input
    }

    size_t len = strlen(rootTopic) + strlen(subtopic) + 2; // +1 for '/' and +1 for '\0'
    char* topic = new char[len]; // Use new char[]

    if (topic == nullptr) { // Check for allocation failure
        Serial.println("Error: Memory allocation failed for topic");
        return nullptr;
    }

    strcpy(topic, rootTopic);
    strcat(topic, "/"); // Add the separator
    strcat(topic, subtopic);

    return topic;
}

// Helper function to concatenate message with date and time
char* mqtt::buildMessage(const char* message) {
    static char msg[100]; // Adjust size as needed
    quando.getTime();
    snprintf(msg, sizeof(msg), "%s - %s - %s", message, quando.getDate(), quando.getHour());
    return msg;
}

//MQTT Loop

void mqtt::mqttLoop(void){
    if (!client->connected()) { mqttReconnect(); }
    client->loop();
}

//MQTT Init
void mqtt::mqttInit(void){
    int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    const char* initialMsg = "ReservOnline";
    if (numCerts == 0) {
        Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
        return; // Can't connect to anything w/o certs!
    }
    BearSSL::WiFiClientSecure* bear = new BearSSL::WiFiClientSecure();
    //Integrate the cert store with this connection
    bear->setCertStore(&certStore);
    mqttInstance = this; // Set the instance in mqttInit. VERY IMPORTANT!
    client = new PubSubClient(*bear);
    client->setServer(mqttServer, mqttPort);
    client->setCallback(mqtt::mqttCallbackStatic); // Use the static bridge function

    while (!client->connected()) {
        
        // Attempt to connect
        if (client->connect(mqttClientId.c_str(), mqttUsername, mqttPassword, buildTopic(lastWillTopic), 0, true, "caiu")) {
        // Once connected, publish an announcement…
        snprintf (msg, MSG_BUFFER_SIZE, initialMsg, value);
        client->publish(buildTopic(isConnectedTopic), initialMsg);
        // … and resubscribe
        client->subscribe(buildTopic(allTopicsReserv)); // Subscribe to all under root
        client->subscribe(allTopicsConsts); //Keep it as is
        } 
    delay(500);
    } 
}

// Static callback bridge
void mqtt::mqttCallbackStatic(char* topic, byte* payload, unsigned int length) {
    if (mqttInstance != nullptr) { // Check if the instance is valid
        mqttInstance->mqttCallback(topic, payload, length); // Call the instance method
    }
}

// Helper function to compare topics
bool mqtt::mqttCallbackCompareTopic(const char* topic, const char* expectedTopic) {
    const char* builtTopic = buildTopic(expectedTopic);
    bool result = false;

    if (builtTopic != nullptr && topic != nullptr) { // Check for null pointers
        size_t builtTopicLen = strlen(builtTopic);

        if (strlen(topic) >= builtTopicLen) {
            result = strncmp(topic, builtTopic, builtTopicLen) == 0;
        }
    }

    delete[] builtTopic; // Free memory (crucial!)

    return result;
}

// Callback function
void mqtt::mqttCallback(char* topic, byte* payload, unsigned int length){
    float compensationFactor;

    if (mqttCallbackCompareTopic(topic, distCompTopic)){       
        float compMsgFloat;
        char* endPtr;  // Important for error checking
        

        compMsgFloat = strtof(reinterpret_cast<const char*>(payload), &endPtr);
        bool conversionFailed = *endPtr != '\0';
        bool zeroButNotZero = (compMsgFloat == 0.0f) && (payload[0] != '0');
        
        if (conversionFailed || zeroButNotZero) { // Check if the entire string was converted or if it is a 0 and payload is not "0"
            Serial.println("Error: Invalid float format in payload");
            // Handle the error (e.g., set a default value, send an error message)
        } else {
            EEPROM.put(distCompensationAddr, compMsgFloat);
            EEPROM.commit();
            EEPROM.get(distCompensationAddr, compensationFactor);
        }    
    }
    else if (mqttCallbackCompareTopic(topic, pumpTopic)) { // Verifica se o tópico é o da bomba
        char message[length + 1]; // Cria um buffer para a mensagem, +1 para o caractere nulo
        for (unsigned int i = 0; i < length; i++) {
            message[i] = (char)payload[i];
        }
        message[length] = '\0'; // Adiciona o caractere nulo ao final da string
        if (strcmp(message, "PUMP_ON") == 0) {
            digitalWrite(SysIni::PUMP, HIGH);
            client->publish(buildTopic(pumpTopic), "cmd ligar bomba recebido");
            // Serial.println("Bomba ligada!");
            
            if(digitalRead(SysIni::PUMP)){
                client->publish(buildTopic(pumpTopic), buildMessage("Bomba ligada com sucesso!"));
                client->publish(buildTopic(pumpTopic), "ligada");
                PUMP_MANUAL=true;
            }
            else{
                client->publish(buildTopic(pumpTopic), "Erro ao ligar bomba!");
                PUMP_MANUAL=false;
            }
            
        } else if(strcmp(message, "PUMP_OFF") == 0) {
            digitalWrite(SysIni::PUMP, LOW);
            client->publish(buildTopic(pumpTopic), "cmd desligar bomba recebido");
            Serial.println("Bomba desligada!");
            if(!digitalRead(SysIni::PUMP)){
                client->publish(buildTopic(pumpTopic), buildMessage("Bomba desligada com sucesso!"));
                client->publish(buildTopic(pumpTopic), "desligada");
                PUMP_MANUAL=false;
            }
            else{
                client->publish(buildTopic(pumpTopic), "Erro ao desligar bomba!");
                PUMP_MANUAL=false;
            }
        }
    }
    else if (mqttCallbackCompareTopic(topic, report)){
        char message[length + 1]; // Cria um buffer para a mensagem, +1 para o caractere nulo
        for (unsigned int i = 0; i < length; i++) {
            message[i] = (char)payload[i];
        }
        message[length] = '\0'; // Adiciona o caractere nulo ao final da string
        if (strcmp(message, "report") == 0) {
            // mqttSendTelWater(!digitalRead(SysIni::PRESS));
            // mqttSendTelFull(!digitalRead(SysIni::BOIA));
            mqttSendReport();
        }   
    }
    else  if (mqttCallbackCompareTopic(topic, flowReadEnableTopic)) {
        // Handle flowReadEnable topic
        char message[length + 1];
        for (unsigned int i = 0; i < length; i++) {
            message[i] = (char)payload[i];
        }
        message[length] = '\0'; // Null-terminate

        if (strcmp(message, "true") == 0) {
            flowMeasurementEnable = true; // Set the atomic variable directly
            Serial.println("Flow measurement enabled via MQTT");
            mqttSendMessage(buildTopic(flowReadEnableTopic),"Flow measurement enabled via MQTT");
        } else if (strcmp(message, "false") == 0) {
            flowMeasurementEnable = false; // Set the atomic variable directly
            Serial.println("Flow measurement disabled via MQTT");
            mqttSendMessage(buildTopic(flowReadEnableTopic),"Flow measurement disabled via MQTT");

        } else {
          Serial.println("Invalid message for flowReadEnable topic");
          mqttSendMessage(buildTopic(flowReadEnableTopic),"Invalid message for flowReadEnable topic");
        }
    }
    else if (mqttCallbackCompareTopic(topic, restart)){
        ESP.restart();
    }
}


//MQTT Reconnect
void mqtt::mqttReconnect(void) {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis(); 
    if (now - lastReconnectAttempt < 5000) { // Don't attempt reconnection too frequently
      return;
    }  
    lastReconnectAttempt = now;
      
    if (client->connect(mqttClientId.c_str(), mqttUsername, mqttPassword, buildTopic(lastWillTopic), 0, true, "caiu")) {
      client->publish(buildTopic(isConnectedTopic), "ReservOnline");
      client->subscribe(buildTopic(allTopicsReserv));
      client->subscribe(allTopicsConsts);
    }
}
//MQTT Senders

void mqtt::mqttSendReport(void) {
    mqttSendTelWater(agua);
    mqttSendTelFull(cheio);
    mqttSendTelPump(bomba);
    mqttSendTelVolReserv(measurementReserv.getVolReserv());
    mqttSendTelDist(measurementReserv.getDistance());
    mqttSendTelDistError(measurementReserv.getVolReservErro());
}

void mqtt::mqttSendTelWater(bool water) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char* topic = buildTopic(waterTopic);
    if (topic != nullptr) { // Check for allocation failure
        if (water) {
            client->publish(topic, "agua");
        } else {
            client->publish(topic, "sem_agua");
        }
        delete[] topic; // Free the memory!
    }
    lastTelemetry = horaStr;
}

void mqtt::mqttSendTelFull(bool full) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char* topic = buildTopic(fullTopic);
    if (topic != nullptr) {
        if (full) {
            client->publish(topic, "cheio");
        } else {
            client->publish(topic, "nao_cheio");
        }
        delete[] topic;
    }
    lastTelemetry = horaStr;
}

void mqtt::mqttSendTelPump(bool pump) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char* topic = buildTopic(pumpTopic);
    if (topic != nullptr) {
        if (pump) {
            client->publish(topic, "ligada");
        } else {
            client->publish(topic, "desligada");
        }
        delete[] topic;
    }
    lastTelemetry = horaStr;
}

void mqtt::mqttSendTelFlow(float flow) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char flowMsg[16];
    sprintf(flowMsg, "%.2f", flow);
    snprintf(msg, MSG_BUFFER_SIZE, flowMsg, value);

    char* topic = buildTopic(flowTopic);
    if (topic != nullptr) {
        client->publish(topic, msg);
        delete[] topic;
    }

    lastTelemetry = horaStr;
}

void mqtt::mqttSendTelVolReserv(float volReserv) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char volReservMsg[16];
    sprintf(volReservMsg, "%.3f", volReserv);
    snprintf(msg, MSG_BUFFER_SIZE, volReservMsg, value);

    char* topic = buildTopic(volResTopic);
    if (topic != nullptr) {
        client->publish(topic, msg);
        delete[] topic;
    }
    lastTelemetry = horaStr;
}

void mqtt::mqttSendTelDist(float dist) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char distMsg[16];
    sprintf(distMsg, "%.2f", dist);
    snprintf(msg, MSG_BUFFER_SIZE, distMsg, value);

    char* topic = buildTopic(distTopic);
    if (topic != nullptr) {
        client->publish(topic, msg);
        delete[] topic;
    }
    lastTelemetry = horaStr;
}

void mqtt::mqttSendTelDistError(int erro) {
    if (!client->connected()) {
        mqttReconnect();
    }
    char erroMsg[16];
    sprintf(erroMsg, "%d", erro);
    snprintf(msg, MSG_BUFFER_SIZE, erroMsg, value);

    char* topic = buildTopic(distErrorTopic);
    if (topic != nullptr) {
        client->publish(topic, msg);
        delete[] topic;
    }
    lastTelemetry = horaStr;
}

void mqtt::mqttSendMessage(const char* topic, const char* message) {
    if (!client->connected()) {
        mqttReconnect();
    }

    char* builtTopic = buildTopic(topic);  // Build the full topic
    if (builtTopic != nullptr) {
        client->publish(builtTopic, message);
        delete[] builtTopic; // Free the memory!
    }
}
//MQTT Senders end

//Getters
int mqtt::getTelemetryFreq() const{ return mqttTelemertyFreq; }
bool mqtt::getPumpManualSet() const{ return PUMP_MANUAL; }

//Setters
void mqtt::setPumpManualSet(bool set) { 
    PUMP_MANUAL = set; 
    mqttSendTelPump(set);
}


// bool mqtt::getFlowReadEnable() const{ return flowMeasurementEnable; }