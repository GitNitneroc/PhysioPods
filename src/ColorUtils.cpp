#include <Arduino.h>
#include "ColorUtils.h"

 uint8_t ColorUtils::hue2rgb(uint8_t p, uint8_t q, uint16_t t) {
    /*Serial.println("p : " + String(p));
    Serial.println("q : " + String(q));
    Serial.println("t : " + String(t)); */
    if (t < 10923) return p + (q - p) * t / 10923;
    if (t < 32768) return q;
    if (t < 43690) return p + (q - p) * (54613 - t) / 10923;
    return p;
}

Color ColorUtils::hslToColor(uint16_t h, uint8_t s, uint8_t l) {
    Color c;

    if (s == 0) {
        c.r = c.g = c.b = l; // achromatic
    } else {
        uint8_t q = l < 128 ? l * (255 + s) / 255 : l + s- (l * s) / 255;
        /* Serial.println("q : " + String(q)); */
        uint8_t p = 2 * l - q;
        /* Serial.println("p : " + String(p)); */
        c.r = hue2rgb(p, q, h + 21845);
        c.g = hue2rgb(p, q, h);
        c.b = hue2rgb(p, q, h - 21845);
    }
    /* Serial.println("rgb(" + String(c.r) + ", " + String(c.g) + ", " + String(c.b)+")"); */
    return c;
}