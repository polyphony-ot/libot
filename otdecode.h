#ifndef libot_otdecode_h
#define libot_otdecode_h

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "hex.h"
#include "cJSON.h"

typedef enum {
    OT_ERR_NONE = 0,
    OT_ERR_PARENT_MISSING = 1,
    OT_ERR_CLIENT_ID_MISSING = 2,
    OT_ERR_COMPONENTS_MISSING = 3,
    OT_ERR_INVALID_COMPONENT = 4,
    OT_ERR_HASH_MISSING = 5
} ot_decode_err;

// Decodes an operation from a UTF-8 JSON string.
ot_decode_err ot_decode(ot_op* op, const char* const json);

#endif
