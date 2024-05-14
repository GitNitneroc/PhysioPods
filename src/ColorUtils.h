#include <Arduino.h>
#ifndef COLORUTILS_H
#define COLORUTILS_H

//TODO : ce fichier ne sert plus à rien, fastLED a déjà une fonction pour convertir du HSL en RGB

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