#include "utf8.h"

size_t utf8_cps(const char byte) {
    if (byte <= 0x7f) {
        return 1;
    } else if (byte <= 0xbf) {
        return -1;
    } else if (byte <= 0xdf) {
        return 2;
    } else if (byte <= 0xef) {
        return 3;
    } else if (byte <= 0xf7) {
        return 4;
    }

    return -1;
}

size_t utf8_length(const char* str) {
    size_t length = 0;
    size_t i = 0;
    while (i < strlen(str)) {
        size_t cps = utf8_cps(str[i]);
        i += cps;
        length++;
    }

    return length;
}

size_t utf8_bytes(const char* str, size_t length) {
    size_t num_bytes = 0;
    size_t num_chars = 0;
    while (num_chars < length) {
        size_t cps = utf8_cps(str[num_bytes]);
        num_bytes += cps;
        num_chars++;
    }

    return num_bytes;
}
