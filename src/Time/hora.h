#ifndef HORA_H
#define HORA_H

#include <arduino.h>


#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define NTP_SERVERS "time4.google.com", "time.google.com"
#define TIME_ZONE -3*ONE_HOUR_IN_SECS


class Hora
{
    public:
        Hora(void);
        void initTimes(void);
        void getTime(void);
        char* getDate(void);
        char* getHour(void);  

    private: 
        void setTimes(void);
        void sunUpdate(void);
        void updateTime(void);
        const long y2k = 946684800;
        
};

#endif // HORA_H