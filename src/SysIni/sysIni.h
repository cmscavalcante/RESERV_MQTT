#ifndef SYSINI_H
#define SYSINI_H

#include <arduino.h>
#include <ezButton.h>

#include <EEPROM.h>
#include <FS.h>
#include <LittleFS.h>


// #include "sysvars.h"

extern ezButton press;
extern ezButton boia;

class SysIni

{
    public:
        // SysIni (void);
        static void init(void);
        // static void inputsLoop(void);
        static const int VAZAO = D1;
        
        
        static const int PUMP = D3;
        static const int VALV_DIR = D8;
        static const int VALV_PUMP = D5;
        static const int LED = D4;

        

  

    private:
        // String dir_path;
        static void hardwareIni(void);
        static void varsIni(void);
        static void fileSysIni(void);
        static void listAllFilesInDir(String dir_path);
 
        
};

#endif // SYSINI_H