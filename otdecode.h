#ifndef libot_otdecode_h
#define libot_otdecode_h

#include "ot.h"

// Decodes an operation from a UTF-8 JSON string.
ot_op* ot_decode(const char* const json);

#endif
