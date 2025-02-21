#include <Arduino.h>

#include <time.h>
#include <Sun.h>

#include "hora.h"

Sun sun(-23.32, -46.54);

//Time vars
    String dayTime, riseStr, setStr, dataStr, horaStr;

    String lastTelemetry="";
    int sunriseHora, sunriseMin, sunriseSeg, sunsetHora, sunsetMin, sunsetSeg=0;
        unsigned long rise, set = 0;
        int hora_anterior, dia_anterior =0;
        int hora, minute, seg, mill, dia, mes, ano =0;
        uint32_t ntpTimeOutMs = 10000;



Hora::Hora(void){};

void Hora::initTimes(void){
    updateTime();
    sunUpdate();
    hora_anterior=hora;
    dia_anterior=dia;
}

char* Hora::getDate(void){
    setTimes();
    char* dateStr = new char[11];
    time_t now=time(NULL)+TIME_ZONE;
    struct tm *tmp = gmtime(&now);
    strftime(dateStr, 11, "%d/%m/%Y", tmp);
    return dateStr;
}

char* Hora::getHour(void){
    setTimes();
    char* hourStr = new char[9];
    time_t now=time(NULL)+TIME_ZONE;
    struct tm *tmp = gmtime(&now);
    strftime(hourStr, 9, "%H:%M:%S", tmp);
    return hourStr;
}



void Hora::getTime(void){
    setTimes();
    time_t now=time(NULL)+TIME_ZONE;
    struct tm* tmp = gmtime(&now);
    dia = tmp->tm_mday;
    mes=tmp->tm_mon+1;
    ano = 1900+tmp->tm_year;
    //Data String
    dataStr = "";
    String mesStr="";
    mesStr.concat(mes);
    String anoStr="";
    anoStr.concat(ano);
    if (dia<10) {
        dataStr = "0";
        dataStr.concat(dia);}
    else {dataStr.concat(dia);}
    dataStr.concat("-");
    if (mes<10) {
        dataStr.concat("0");
        dataStr.concat(mes);}
    else {dataStr.concat(mes);};
    dataStr.concat("-");
    dataStr.concat(ano);
    //Hora String
    hora=tmp->tm_hour;
    minute=tmp->tm_min;
    seg=tmp->tm_sec;
    horaStr = "";
    if (hora<10) {
        horaStr = "0";
        horaStr.concat(hora);}
    else {horaStr=hora;}
    horaStr.concat(":");
    if (minute<10) {
        horaStr.concat("0");
        horaStr.concat(minute);}
    else {horaStr.concat(minute);}
    horaStr.concat(":");
    if (seg<10) {
        horaStr.concat("0");
        horaStr.concat(seg);}
    else {horaStr.concat(seg);}
    Serial.print(dataStr+" - ");
    Serial.println(horaStr);
}

void Hora::setTimes(void){
    if (hora_anterior!=hora){
        updateTime();
        hora_anterior=hora;
    }
    if (dia_anterior!=dia){
        sunUpdate();
        dia_anterior=dia;
    }
}

void Hora::sunUpdate(void){
    time_t now=time(NULL);
    rise = sun.getRise(now)-TIME_ZONE;
    set = sun.getSet(now)-TIME_ZONE;
    rise=rise-y2k;
    set=set-y2k;

    const time_t rise_t=rise-10800;
    riseStr = String(ctime(&rise_t));
    riseStr = riseStr.substring(11,19);
    String riseHoraStr = riseStr.substring(0,2);
    sunriseHora=riseHoraStr.toInt();
    String riseMinStr = riseStr.substring(3,5);
    sunriseMin=riseMinStr.toInt();
    String riseSegStr = riseStr.substring(6,8);
    sunriseSeg=riseSegStr.toInt();
    riseStr = riseStr.substring(0,5);

    const time_t set_t=set-10800;
    setStr = String(ctime(&set_t));
    setStr = setStr.substring(11,19);
    String setHoraStr = setStr.substring(0,2);
    sunsetHora=setHoraStr.toInt();
    String setMinStr = setStr.substring(3,5);
    sunsetMin=setMinStr.toInt();
    String setSegStr = setStr.substring(6,8);
    sunsetSeg=setSegStr.toInt();
    setStr = setStr.substring(0,5);
    Serial.println("Alvorada: "+riseStr+"; Ocaso: "+setStr);
}

void Hora::updateTime(void){
    Serial.print("Setting time using SNTP");
    configTime(TIME_ZONE, 0, NTP_SERVERS);
    time_t now = time(NULL);
    uint32_t timeOut=0;
    int tries = 0;
    while (tries < 10){
        while (now < 1510592825)
        {
            Serial.print(".");
            now = time(NULL);
            timeOut++;
            if (timeOut > ntpTimeOutMs/500){
            Serial.print("Time not set. Server unreachable. Try #");
            Serial.println(tries);
            break;
            }
        }
        delay(1000);
        tries++;
    }
    Serial.println("done!");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.printf("%s", asctime(&timeinfo));
}
