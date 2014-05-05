#ifndef libot_otencode_h
#define libot_otencode_h

#include <inttypes.h>
#include "ot.h"

// Encodes an operation as a UTF-8 JSON string.
char* ot_encode(const ot_op* const op);

#endif
