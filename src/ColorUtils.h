#include <Arduino.h>
#ifndef COLORUTILS_H
#define COLORUTILS_H


struct Color{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

namespace ColorUtils {
    
    uint8_t hue2rgb(uint8_t p, uint8_t q, uint16_t t) ;
    Color hslToColor(uint16_t h, uint8_t s, uint8_t l);
}

#endif //COLORUTILS_H