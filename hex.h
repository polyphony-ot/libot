#ifndef libot_hex_h
#define libot_hex_h

#include <stdint.h>

// Provides functions for encoding and decoding hex strings.

// Converts a hex string to an array. The hex string is decoded and copied into
// a. a must have at least len/2 space allocated to hold the hex value. len is
// the length of the hex string.
//
// Warning: This function does not perform any sort of input validation, so it's
// possible to pass in an invalid hex string and get an undefined result.
int hextoa(uint8_t* a, const uint8_t* hex, size_t len);

// Converts an array to a hex string. The array is encoded and copied into hex.
// hex must have at least len*2 space allocated to hold the encoded hex value.
// len is the length of the array.
int atohex(uint8_t* hex, const uint8_t* a, size_t len);

#endif
