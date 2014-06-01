#ifndef libot_otdecode_h
#define libot_otdecode_h

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "hex.h"
#include "cJSON.h"

// Decodes an operation from a UTF-8 JSON string.
ot_err ot_decode(ot_op* op, const char* const json);

#endif
