//main.cpp

#include <Arduino.h>


  // C99 libraries
  #include <cstdlib>
  #include <stdio.h>
  #include <stdbool.h>
  #include <float.h>
  #include <atomic>

  #include <ezButton.h>

  //Delays
  #include <AsyncDelay.h>

  //Libraries
  #include "sysvars.h"
  #include "MQTT\mqttSys.h"
  #include "SysIni\sysIni.h"
  #include "Time\hora.h"
  #include "WiFi\wifiIni.h"
  #include "OTA\OTA.h"
  #include "Measurements\flow.h"
  #include "medReserv\medReserv.h"
  #include "interruptControl.h"

//Declare Functions
  void IRAM_ATTR vazaoInterrup(void);
  void setupSys(void);

  static const int PRESS = D2;
  static const int BOIA = D0;

  ezButton press(PRESS, INPUT_PULLUP);
  ezButton boia(BOIA, INPUT_PULLUP);

//Instances
  //delays
    AsyncDelay oneSecDelay;
    AsyncDelay measureDelay;
    AsyncDelay ledBlinkDelay;
    AsyncDelay levelMeasurementDelay;
    AsyncDelay flowDelay;

  //Constructors for the libraries
    Hora tempo;
    wifiIni wifiInit;
    otaHandle OTA_handle;
    Flow flowIn;
    medReserv medVol(10.0f, 200.0f, 5000.0f);
    mqtt Mqtt(medVol);
    SysIni sysIni;

bool cheio, bomba=false;
bool agua = true;

std::atomic<bool> flowMeasurementEnable=false;
bool mqttHalter=false;

inline const char * const BoolToString(bool b)
{
  return b ? "true" : "false";
}

void setupSys(void){ //init all system vars during setup
  medVol.init();
  measureDelay.start(30000, AsyncDelay::MILLIS);
  levelMeasurementDelay.start(Mqtt.getTelemetryFreq(), AsyncDelay::MILLIS);
  ledBlinkDelay.start(2000, AsyncDelay::MILLIS);
  oneSecDelay.start(1000, AsyncDelay::MILLIS);
  flowDelay.start(5000, AsyncDelay::MILLIS);
  press.loop();
  agua=press.getState();
  boia.loop();
  cheio=!boia.getState();
  bomba=false;
  medVol.medVolume();
  delay(500);
  Mqtt.mqttSendReport();
  attachVazaoInterrupt();
}

void attachVazaoInterrupt() {
  attachInterrupt(digitalPinToInterrupt(SysIni::VAZAO), vazaoInterrup, RISING);
}

void detachVazaoInterrupt() {
  detachInterrupt(digitalPinToInterrupt(SysIni::VAZAO));
}

void IRAM_ATTR vazaoInterrup(){ //Atualiza fluxo de água de entrada
  if(flowMeasurementEnable){ flowIn.flowPulseChange(); }
}


//setup and loop
void setup(){
  Serial.begin(74880);
  delay(500);
  sysIni.init();
  press.setDebounceTime(50);
  boia.setDebounceTime(50); 
  wifiInit.wifiInit();
  wifiInit.connectToWiFi(); //WiFi
  tempo.initTimes();
  Mqtt.mqttInit(); //MQTT
  otaHandle::otaInit(Mqtt); //OTA 
  setupSys();
}

void loop(){
  bool agua_ant=agua;
  bool cheio_ant=cheio;
  bool bomba_ant=bomba;
  bool flowMeasurementEnable_ant=flowMeasurementEnable;
  ArduinoOTA.handle();
  if(!mqttHalter){
    Mqtt.mqttLoop();
    if (flowMeasurementEnable_ant != flowMeasurementEnable){
      if (flowMeasurementEnable){
        char topic[10];
        char msg[100];
        snprintf(topic, sizeof(topic), "flow");
        snprintf(msg, sizeof(msg), "Flow read enabled");
        Mqtt.mqttSendMessage(topic, msg);
      }
      else{
        char topic[10];
        char msg[100];
        snprintf(topic, sizeof(topic), "flow");
        snprintf(msg, sizeof(msg), "Flow read disabled");
        Mqtt.mqttSendMessage(topic, msg);
      }
    }
    if (flowMeasurementEnable) {
        if (flowDelay.isExpired()) {
            float flowRate = flowIn.flowCalc(); // Calculate flow rate ALWAYS
            if (flowIn.flowIsChanged()) {      // Check if the flow rate has changed
                Mqtt.mqttSendTelFlow(flowRate); // Send the flow rate (even if zero)
            }
            flowDelay.repeat();
        }
    }
    //Read inputs
    press.loop();
    agua=press.getState();
    boia.loop();
    cheio=!boia.getState();
    if (agua!=agua_ant){ Mqtt.mqttSendTelWater(agua); }
    if (cheio!=cheio_ant){ Mqtt.mqttSendTelFull(cheio); }
    if (!Mqtt.getPumpManualSet()){
      if (agua && !cheio && !bomba){
        digitalWrite(sysIni.PUMP, HIGH);
        bomba=true;
        if(bomba!=bomba_ant){ Mqtt.mqttSendTelPump(bomba); }
      }
      if ((!agua) || (cheio)){
        digitalWrite(sysIni.PUMP, LOW);
        bomba=false;
        if(bomba!=bomba_ant){ Mqtt.mqttSendTelPump(bomba); }
      }

    }
    else{
      if (cheio){
        digitalWrite(sysIni.PUMP, LOW);
        bomba=false;
        if(bomba!=bomba_ant){ Mqtt.mqttSendTelPump(bomba); 
        Mqtt.setPumpManualSet(bomba);}
      }
    }
    if (levelMeasurementDelay.isExpired()){
      medVol.medVolume();
      if (medVol.getVolReservErro() == 0){
        Mqtt.mqttSendTelVolReserv(medVol.getVolReserv());
        Mqtt.mqttSendTelDist(medVol.getDistance());
      }
      else{ Mqtt.mqttSendTelDistError(medVol.getVolReservErro()); }
      levelMeasurementDelay.repeat();
    }   
  }
    
    
  if (oneSecDelay.isExpired()){
    digitalWrite(SysIni::LED, !digitalRead(SysIni::LED));   
    oneSecDelay.repeat();
  }
}