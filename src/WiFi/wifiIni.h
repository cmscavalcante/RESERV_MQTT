#ifndef WIFI_INI_H
#define WIFI_INI_H

#include <Arduino.h>


class wifiIni
{
    public:
        wifiIni(void);
        bool wifiInit(void);
        bool connectToWiFi(void);
    private:
        // char _hostName;       
};

#endif //WIFI_INI_H