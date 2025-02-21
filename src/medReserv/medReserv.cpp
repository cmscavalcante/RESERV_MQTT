#include <Arduino.h>
#include <math.h>

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "SysIni/sysIni.h"
#include "medReserv.h"




#define SERIAL_RX D6
#define SERIAL_TX D7

medReserv::medReserv(float hGap, float hReserv, float volTotalReserv) : 
  hGap(hGap), hReserv(hReserv), volTotalReserv(volTotalReserv),
  volReserv(0.0f), distance(0.0f), distCompensation(0.0f), volReservChanged(false), volReservErro(0) { sReserv = volTotalReserv/hReserv;
  }
  
  

SoftwareSerial ultraSound(SERIAL_RX, SERIAL_TX);

void medReserv::init(void){
    pinMode(SERIAL_RX, INPUT);
    pinMode(SERIAL_TX, OUTPUT);
    ultraSound.begin(uSbaudRate, SWSERIAL_8N1);
    EEPROM.get(0, distCompensation);
}

void medReserv::medVolume(void){ //Mede nivel reservatorio
    byte startByte, highByte, lowByte, sum = 0;
    byte buf[3];
    float volReservAnt = volReserv;
    ultraSound.write(0x01);
    if(ultraSound.available()){
      startByte = (byte)ultraSound.read();
      if(startByte == 255){
        ultraSound.readBytes(buf, 3);
        highByte = buf[0];
        lowByte = buf[1];
        sum = buf[2];
        distance = (highByte<<8) + lowByte;
        if(((0xFF + highByte + lowByte) & 0x00FF) !=sum ){
          distance = 0;
        }
      }
      if (distance == 0){
        volReservErro=2;//ultrasound error
      }
      else{
        volReservErro=0;//no error
        distance=distance/10;//convert mm to cm
        volReserv=((hReserv-distance+hGap)*sReserv)/lTOm3;//calc volume in m3
        if (abs(volReserv - volReservAnt) > 0.01){ //0.1m3 threshold
          volReservChanged=true;}
      }
    }
    else{
      volReservErro=1;//ultrasound not available
    }
  }

// Getter implementations
float medReserv::getVolReserv() const { return volReserv; }
bool medReserv::getVolReservChanged() const { return volReservChanged; }
int medReserv::getVolReservErro() const { return volReservErro; }
float medReserv::getDistance() const { return distance; }

// Setter implementations (if needed)
void medReserv::setVolReservChanged(bool changed) { volReservChanged = changed; }
void medReserv::setVolReservErro(int erro) { volReservErro = erro; }
void medReserv::setDistCompensation(float value) { distCompensation = value; }
  
