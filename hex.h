#ifndef libot_hex_h
#define libot_hex_h

#include <stdint.h>

// Converts a hex string to an array. The hex string is decoded and copied into
// a. a must have at least len/2 space allocated to hold the hex value. len is
// the length of the hex string.
int hextoa(uint8_t* a, uint8_t* hex, size_t len);

// Converts an array to a hex string. The array is encoded and copied into hex.
// hex must have at least len*2 space allocated to hold the encoded hex value.
// len is the length of the array.
int atohex(uint8_t* hex, uint8_t* a, size_t len);

#endif
