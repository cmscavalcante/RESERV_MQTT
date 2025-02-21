#ifndef MEDRESERV_H
#define MEDRESERV_H

#include <Arduino.h>

class medReserv
{
    public:
        medReserv(float hGap, float hReserv, float volTotalReserv);
        void medVolume(void);
        void init(void);

        // Getters (Accessors)
        float getVolReserv() const;
        bool getVolReservChanged() const;
        int getVolReservErro() const;
        float getDistance() const;
        // Setters (Mutators) - If needed, but use with caution!
        void setVolReservChanged(bool changed);  // Only if you need to set from outside
        void setVolReservErro(int erro);        // Only if you need to set from outside
        void setDistCompensation(float value);
        
       
    private:
        float hGap;
        float hReserv;
        float volTotalReserv;
        float volReserv;
        float distance;
        float distCompensation;
        bool volReservChanged;
        int volReservErro;

        float sReserv;

        const float lTOm3 = 1000;
        const uint32_t uSbaudRate = 9600;
        
};

#endif // MEDRESERV_H