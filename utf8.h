#ifndef LIBOT_UTF8_H
#define LIBOT_UTF8_H

#include <inttypes.h>
#include <string.h>

size_t utf8_cps(const char byte);

size_t utf8_length(const char* str);

size_t utf8_bytes(const char* str, size_t length);

#endif
