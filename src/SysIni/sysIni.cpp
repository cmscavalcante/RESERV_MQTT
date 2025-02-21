#include <Arduino.h>

#include <EEPROM.h>
#include <FS.h>
#include <LittleFS.h>
//Debounce


#include "sysIni.h"



extern int distCompensationAddr, EEPROM_SIZE;
extern float distCompensation;
extern bool PUMP_ON;
extern float volume, vol_ant;
extern bool volInChanged, volReservChanged, volReservErro;


// SysIni::SysIni (void){}

void SysIni::init(void){
    hardwareIni();
    varsIni();
    fileSysIni();
}

void SysIni::hardwareIni(void){

    pinMode(VAZAO, INPUT_PULLUP);
    
    
    pinMode(LED, OUTPUT);
    pinMode(PUMP, OUTPUT);
    pinMode(VALV_DIR, OUTPUT);
    pinMode(VALV_PUMP, OUTPUT);
    digitalWrite(PUMP, LOW);
    digitalWrite(VALV_DIR, LOW);
    digitalWrite(VALV_PUMP, LOW);
    // EEPROM.begin(512);
    EEPROM.begin(EEPROM_SIZE);
}

void SysIni::varsIni(void){
    PUMP_ON=false;
    volume=0;
    vol_ant=volume;
    // volReservChanged=false;
    // volReservErro=false;
    // volInChanged=false;
    //EEPROM.get(distCompensationAddr, distCompensation);    
}

void SysIni::fileSysIni(void){
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
        Serial.println("LittleFS Mounted");
        Serial.println("List of files in FS:");
        listAllFilesInDir("/");
    }
}

void SysIni::listAllFilesInDir(String dir_path){//Check if cert files are in path
    Dir dir = LittleFS.openDir(dir_path);
    while(dir.next()) {
        if (dir.isFile()) {
        // print file names
        Serial.print("File: ");
        Serial.println(dir_path + dir.fileName());
        Serial.println("");
        }
        if (dir.isDirectory()) {
        // print directory names
        Serial.print("Dir: ");
        Serial.println(dir_path + dir.fileName() + "/");
        // recursive file listing inside new directory
        listAllFilesInDir(dir_path + dir.fileName() + "/");
        }
    }
}

// void SysIni::inputsLoop(void){
//     press.loop();
//     boia.loop();
// }



