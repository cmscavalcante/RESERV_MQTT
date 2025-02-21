#ifndef SYSVARS_H
#define SYSVARS_H
#include "iot_configs.h"
#include "time\timeVars.h"





//EEPROM
int EEPROM_SIZE = 512;
int distCompensationAddr = 0;

//Controle bomba
bool PUMP_ON;
//char *estado;

float volume, vol_ant;
float litros;
// float flow;



bool volInChanged;
// bool flowChanged;
    



#endif //SYSVARS_H