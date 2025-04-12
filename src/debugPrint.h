//Webserial
#include <WebSerial.h>

#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#ifdef isDebug //In debug builds we use both Serial and WebSerial, it doesn't matter if WebSerial is not started yet
    #define DebugPrint(x) { Serial.print(x); WebSerial.print(x); }
    #define DebugPrintln(x) { Serial.println(x); WebSerial.println(x); }
    #define DebugPrintf(x, ...) { Serial.printf(x, __VA_ARGS__); WebSerial.printf(x, __VA_ARGS__); }
    #define DebugWrite(x, y) { Serial.write(x, y); WebSerial.write(x, y); }
#else
    #define DebugPrint(x) Serial.print(x)
    #define DebugPrintln(x) Serial.println(x)
    #define DebugPrintf(x, ...) Serial.printf(x, __VA_ARGS__)
    #define DebugWrite(x, y) Serial.write(x, y)
#endif

#endif // DEBUG_PRINT_H