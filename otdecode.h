#ifndef libot_otdecode_h
#define libot_otdecode_h

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "hex.h"
#include "cjson/cJSON.h"
#include "doc.h"

// Decodes an operation from a UTF-8 JSON string.
ot_err ot_decode(ot_op* op, const char* const json);

// ot_decode_doc decodes a document from a UTF-8 JSON string.
ot_err ot_decode_doc(ot_doc* doc, const char* const json);

#endif
