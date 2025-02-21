#include <Arduino.h>

#include "wifiParam.h"

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
// #include <ESPmDNS.h>
#include <time.h>

#include "wifiIni.h"
#include "wifiParam.h"

ESP8266WiFiMulti wifiM;

IPAddress subnet    (WIFI_SUBNET);
IPAddress gateway   (WIFI_GATEWAY);
IPAddress local_IP  (WIFI_LOCAL_IP);
IPAddress dns1      (WIFI_DNS_PRI);
IPAddress dns2      (WIFI_DNS_SEC);

wifiIni::wifiIni(void){}

bool wifiIni::wifiInit(void){
    // Remove static IP configuration
    WiFi.hostname(hostName); //Set hostname before connecting
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true); 
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    wifiM.addAP(ssid1, password1);
    wifiM.addAP(ssid2, password2);
    return(true); // Always return true here, connection status is checked in connectToWiFi
}

bool wifiIni::connectToWiFi(void){
    if (wifiM.run(connectTimeoutMs) == WL_CONNECTED) {
        Serial.print("WiFi connected: ");
        Serial.print(WiFi.SSID());
        Serial.print(" ");
        Serial.println(WiFi.localIP());
        return(true);
        
    } 
    else {Serial.println("WiFi not connected");
        return(false);
    }
}




