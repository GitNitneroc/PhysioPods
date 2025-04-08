//Webserial
#include <WebSerial.h>

#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#ifdef isDebug
    #define DebugPrint(x) { Serial.print(x); WebSerial.print(x); }
    #define DebugPrintln(x) { Serial.println(x); WebSerial.println(x); }
    #define DebugPrintf(x, ...) { Serial.printf(x, __VA_ARGS__); WebSerial.printf(x, __VA_ARGS__); }    
#else
    #define DebugPrint(x) WebSerial.print(x)
    #define DebugPrintln(x) WebSerial.println(x)
    #define DebugPrintf(x, ...) WebSerial.printf(x, __VA_ARGS__)
#endif

#endif // DEBUG_PRINT_H