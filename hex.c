#include "hex.h"

int hextoa(uint8_t* a, const uint8_t* hex, size_t len) {
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        char c;
        if (hex[i] >= 0x61) {           // Lowercase letter
            c = (hex[i] - 0x57) << 4;
        } else if (hex[i] >= 0x41) {    // Uppercase letter
            c = (hex[i] - 0x37) << 4;
        } else {                        // Number
            c = (hex[i] - 0x30) << 4;
        }
        
        ++i;
        
        if (hex[i] >= 0x61) {
            c += hex[i] - 0x57;
        } else if (hex[i] >= 0x41) {
            c += hex[i] - 0x37;
        } else {
            c += hex[i] - 0x30;
        }
        
        a[j] = c;
        ++j;
    }
    
    return 0;
}